class SchanaPartyPositions {
    protected ref map<string, vector> positions;

    void SchanaPartyPositions () {
        positions = new ref map<string, vector> ();
    }

    ref map<string, vector> Get () {
        return positions;
    }

    void Replace (ref array<string> ids, ref array<vector> server_positions) {
        positions.Clear ();
        for (int i = 0; i < ids.Count (); ++i) {
            positions.Insert (ids.Get (i), server_positions.Get (i));
        }
    }
}