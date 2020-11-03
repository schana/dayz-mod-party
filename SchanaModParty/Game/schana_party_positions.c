class SchanaPartyPositions {
    private ref map<string, vector> positions;

    void SchanaPartyPositions () {
        positions = new ref map<string, vector> ();
    }

    ref map<string, vector> Get () {
        return positions;
    }

    void Replace (ref array<string> ids, ref array<vector> server_positions) {
        auto new_positions = new ref map<string, vector> ();
        for (int i = 0; i < ids.Count (); ++i) {
            new_positions.Insert (ids.Get(i), server_positions.Get(i));
        }
        positions = new_positions;
    }
}