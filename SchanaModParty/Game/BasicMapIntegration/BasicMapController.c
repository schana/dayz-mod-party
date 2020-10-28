#ifdef BASICMAP
modded class BasicMapController {
    override void OnMarkerSave (ref BasicMapMarker marker) {
        switch (marker.GetGroup ()) {
            case SchanaPartyBasicMapAPI.GROUP_ID_LOCAL:
                break;
            default:
                break;
        }
        super.OnMarkerSave (marker);
    }

    override void OnMarkerDelete (ref BasicMapMarker marker) {
        switch (marker.GetGroup ()) {
            case SchanaPartyBasicMapAPI.GROUP_ID_LOCAL:
                break;
            case SchanaPartyBasicMapAPI.GROUP_ID_REMOTE:
                break;
            default:
                break;
        }
        super.OnMarkerDelete (marker);
    }
}
#endif