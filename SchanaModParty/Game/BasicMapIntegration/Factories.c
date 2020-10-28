#ifdef BASICMAP

class SchanaPartyLocalMarkerFactory extends BasicMapMarkerFactory {
    override ref BasicMapMarker Create (string name, vector position) {
        BasicMapMarker marker = super.Create (name, position);
        marker.SetGroup (SchanaPartyBasicMapAPI.GROUP_ID_LOCAL);
        return marker;
    }
}

class SchanaPartyRemoteMarkerFactory extends BasicMapMarkerFactory {
    override ref BasicMapMarker Create (string name, vector position) {
        BasicMapMarker marker = super.Create (name, position);
        marker.SetGroup (SchanaPartyBasicMapAPI.GROUP_ID_REMOTE);
        marker.SetCanEdit (false);
        return marker;
    }
}

#endif