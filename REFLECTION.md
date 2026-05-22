# Lab Reflection

## What was the hardest concept this lab?

Honestly, `fork()` messed with my head at first. I thought it would just create
a child process and move on — but then the output printed twice and I sat there
like "wait, what just happened?" The fact that both parent and child continue
from the same point after fork() is something I had to run like 4 times before
it actually made sense to me.

Also the `wait()` and `exec()` combo — why do you need both? I get it now, but
in the moment it felt like too many moving parts at once.

## What clicked unexpectedly?

`strace`. I did not expect to actually *see* the system calls happening in real
time. Like I always knew "programs talk to the kernel" but seeing `execve`,
`write`, `exit_group` just sitting there in the terminal — that was actually
cool. It made the whole "OS manages everything" idea feel real instead of just
something written in a textbook.

Also the `-p` flag in `mkdir -p`. Small thing but I went and looked it up myself
after the lab. Turns out it just means "make parent directories too, and don't
complain if it already exists." Wish someone told me that earlier lol.

## Which project ideas do I want to attempt later?

The mini shell from Lab 4 looks genuinely fun. Not gonna lie I skimmed it during
Lab 1 and thought "no way I can build that" — but now after doing fork + exec +
wait, I kind of see how it fits together. I want to try adding the pipe `|`
support because after seeing how `ls | wc -l` works internally, I really want to
build that myself.

Maybe also the N producers / M consumers thing from Lab 3. I half-understood it
during the lab. I want to go back and actually think through the "how does the
consumer know when to stop" problem on my own.
