
TEST_CASE("Testing sort_by_procs", "[call]") {
    set<Call> calls;
    calls.insert(Call(1, 10, Envelope()));
    calls.insert(Call(1, 5, Envelope()));
    calls.insert(Call(0, 2, Envelope()));

    auto procs = sort_by_procs(calls);
    auto proc1 = procs[0];
    auto it = proc1.begin();
    REQUIRE(it != proc1.end());
    REQUIRE((*it) == Call(0, 2, Envelope()));
    it++;
    REQUIRE(it == proc1.end());

    auto proc2 = procs[1];
    it = proc2.begin();
    REQUIRE(it != proc2.end());
    REQUIRE((*it) == Call(1, 5, Envelope()));
    it++;
    REQUIRE(it != proc2.end());
    REQUIRE((*it) == Call(1, 10, Envelope()));
    it++;
    REQUIRE(it == proc2.end());

    auto proc3 = procs[10];
    REQUIRE(proc3.size() == 0);
}
