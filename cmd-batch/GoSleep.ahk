
DllCall("PowrProf\SetSuspendState", "Int",0, "Int",0, "Int",0)

/*
https://learn.microsoft.com/en-us/windows/win32/api/powrprof/nf-powrprof-setsuspendstate

BOOLEAN SetSuspendState(
  [in] BOOLEAN bHibernate, // 0:sleep , 1:hibernate
  [in] BOOLEAN bForce,
  [in] BOOLEAN bWakeupEventsDisabled
);

/*

