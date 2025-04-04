scriptName MP_HazTheCompletionizt_Quest extends Quest

event OnInit()
    SetObjectiveDisplayed(0, true)
    MP_HazTheCompletionizt.UpdateJournalWithLatestDiscoverableLocationInfo()
endEvent
