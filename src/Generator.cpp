#include "Generator.hpp"
#include "Checker.hpp"

Generator::Generator(const CallDB &db) : db(db) {}

MatchSet Generator::matchCollective() const {
    // assume all are barrier calls ready to be issued
    return MatchSet(db.findCollective());
}

MatchSet Generator::matchReceive() const {
    MatchSet result;
    for (auto recv : db.findReceive()) {
        if (auto send = db.matchReceive(recv)) {
            result.add(*send);
            result.add(recv);
        }
    }
    return result;
}

MatchSet Generator::matchWait() const {
    return MatchSet(db.findWait());
}

vector<MatchSet> add_prefix(const MatchSet match, const vector<MatchSet> matches) {
    if (match.empty()) {
        return matches;
    }
    if (matches.empty()) {
        vector<MatchSet> result;
        result.push_back(match);
        return result;
    }
    vector<MatchSet> result;
    for (auto other : matches) {
        other.prepend(match);
        result.push_back(other);
    }
    return result;
}

vector<MatchSet> mix(const vector<MatchSet> left, const vector<MatchSet> right) {
    if (left.empty()) {
        return right;
    }
    if (right.empty()) {
        return left;
    }
    vector<MatchSet> result;
    for (auto elem : left) {
        auto prefixed = add_prefix(std::move(elem), right);
        result.insert(result.end(), prefixed.begin(), prefixed.end());
    }
    return result;
}

vector<MatchSet> Generator::matchReceiveAny() const {
    vector<MatchSet> result;
    for (auto recv : db.findReceiveAny()) {
        vector<MatchSet> receive;
        for (auto send : db.matchReceiveAny(recv)) {
            MatchSet ms;
            recv.recv.src = send.pid;
            ms.add(recv);
            ms.add(send);
            receive.push_back(ms);
        }
        result = mix(receive, result);
    }
    return result;
}

vector<MatchSet> Generator::getMatchSets() const {
    MatchSet ms;
    ms.append(std::move(matchCollective()));
    ms.append(std::move(matchReceive()));
    ms.append(std::move(matchWait()));
    vector<MatchSet> result;
    result.push_back(ms);
    return ms.empty() ? matchReceiveAny() : result;
}

vector<MatchSet> get_match_sets(const Schedule & schedule) {
    CallDB db(schedule); // make sure the DB is checked
    return Generator(db).getMatchSets();
}
