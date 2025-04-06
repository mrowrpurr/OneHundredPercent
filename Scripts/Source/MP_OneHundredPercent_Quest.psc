scriptName MP_OneHundredPercent_Quest extends Quest

event OnInit()
    SetObjectiveDisplayed(0, true)
    MP_OneHundredPercent.UpdateJournalWithLatestDiscoverableMapMarkers()
endEvent
