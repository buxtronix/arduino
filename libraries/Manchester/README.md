
This is a highly efficient state machine for processing Manchester encoded data.

Rather than long and error prone logic, a simple state machine is used, and
a single line of code actually performs the logic here by way of a state
table.

It is quite simple to use.

First, call m_init(encoding). the encodings are defined in manchester.h.

Next, you feed the state machine inputs based on what the latest event on the
input stream is. The input events indicate whether what was received was a long
or short mark or space, and valid values are defined in manchester.h. It is up
to your application to determine the polarity and duration of the event.

After you pass this to m_advance(), the return value indicates the output state
of the machine. This could be nothing (indicating no definitive new data yet),
or an error, or whether a zero or one bit was emitted.

Copyright 2014 - bbuxton@cactii.net
Licenced under the GNU GPL version 2.

