folder "result" contains the three "a", "b". "c" cases mentioned in wechat.
folder "code" contains the original code of the project.
The headfiles according to the three cases are contained in result.

Two main changes:
1. We do not drop any task when it is unschedulable, instead, we put it on core 3.
2. When setting up timer for the first subtask, if the expected time is less than the current time, we do not ignore this first subtask. If we ignore it, it' parents will be stop forever since we using hrtimer_start instead of hrtimer_forward. Instead, we immediately wake up it.