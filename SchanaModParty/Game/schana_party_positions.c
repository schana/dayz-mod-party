class SchanaPartyPositions {
    private ref map<ref string, ref vector> positions;

    void SchanaPartyPositions () {
        positions = new ref map<ref string, ref vector> ();
    }

    ref map<ref string, ref vector> Get () {
        return positions;
    }

    void Replace (ref array<ref string> ids, ref array<ref vector> server_positions) {
        auto new_positions = new ref map<ref string, ref vector> ();
        for (int i = 0; i < ids.Count (); ++i) {
            new_positions.Insert (ids[i], server_positions[i]);
        }
        positions = new_positions;
    }
}