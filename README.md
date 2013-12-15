malcolms-device
===============

A pedagogical Linux device driver based on the /dev/yes, /dev/no, /dev/maybe joke.

Original man page <http://tonkersten.com/2010/10/75-being-bored/>

Many of the features from the original man page are humorously chimerical and required a somewhat creative interpretation in order to implement them as working features.

what it actually does
=======

/dev/yes will output "yes" then "Yes" then "YES" then start again at "yes" when read.

/dev/no does the same but with the sequence "no", "No", NO".

/dev/maybe will output "yes" or "no" at random.

Several options can be set when loading the module with module_param. These include:

* opt_i -- "Case insensitive", always outputs UPPER case.
* opt_q -- "Quite", no output.
* opt_v -- "Verbose", no no output.
* opt_s [int] -- Only output every given number of seconds. (When read is called inbetween these intervals, nothing is returned.)

The following device files are created for both yes and no:

 * /dev/yes
    The master yes device
 * /dev/yes.1 
    The non rewinding yes device *Will always output the first value in the sequence i.e. lower case.*
 * /dev/yes.s
    The secure yes device, nobody may read it, not even root *-q*
 * /dev/yes.r
    The random yes device, outputs random strings of yes *Each read will return a yes from a random position in the sequence i.e. yesYESYESYesyes etc.*
 * /dev/yes.i
    The ignore case yes device *Upper case*
 * /dev/yes.u
    The uppercase yes device *Upper case*
 * /dev/yes.l
    The lowercase yes device *Lower case, yes.1*
 * /dev/yes.s
    The silent yes device *-q*
 * /dev/yes.no
    The inverted yes device, says yes and means no. This is mostly a link to /dev/no or to the woman device (/dev/woman) *yes.l*
 * /dev/woman
    *yes.l*
 * /dev/yes.marriage
    This is a POSIX 4.1 extension, making the yes device output the text i do *Outputs " i do"
 * /dev/yes.z
    *Opens /dev/mem with flip_open and writes output there. Upon write error the f_pos is forced forwards to attempt to write as much as possible. Unfortunately only the first megabyte or so of /dev/mem is writeable so this will not crash the machine. It does cause strange graphical effects within X though.*
