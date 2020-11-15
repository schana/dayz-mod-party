#ifdef BASICMAP

class SchanaPartyBasicMapMarkerBaseFactory extends BasicMapMarkerFactory {
    override ref BasicMapMarker Create (string name, vector position) {
        BasicMapMarker marker = super.Create (name, position);
        marker.SetIcon ("BasicMap\\gui\\images\\player.paa");
        marker.Colour.Set (0, 33);
        marker.Colour.Set (1, 233);
        marker.Colour.Set (2, 255);
        return marker;
    }
}

class SchanaPartyLocalMarkerFactory extends SchanaPartyBasicMapMarkerBaseFactory {
    override ref BasicMapMarker Create (string name, vector position) {
        BasicMapMarker marker = super.Create (name, position);
        marker.SetGroup (SchanaPartyBasicMapAPI.GROUP_ID_LOCAL);
        return marker;
    }
}

class SchanaPartyRemoteMarkerFactory extends SchanaPartyBasicMapMarkerBaseFactory {
    override ref BasicMapMarker Create (string name, vector position) {
        BasicMapMarker marker = super.Create (name, position);
        marker.SetGroup (SchanaPartyBasicMapAPI.GROUP_ID_REMOTE);
        return marker;
    }
}

class SchanaPartyMemberMarkerFactory extends SchanaPartyBasicMapMarkerBaseFactory {
    override ref BasicMapMarker Create (string name, vector position) {
        BasicMapMarker marker = super.Create (name, position);
        marker.SetGroup (SchanaPartyBasicMapAPI.GROUP_ID_MEMBERS);
        return marker;
    }
}

#endif