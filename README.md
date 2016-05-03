# chtime-win32
Windows (Win32) command-line utility to change the modification timestamps of one or more files.

<pre>
<b>Usage:</b>  <b>chtime</b> [-<i>option</i>...] <i>file</i>...

<b>Options:</b>
    <b>-c</b>              Create new files if they do not already exist.
    <b>-d</b> <i>DD</i>           Change the day of the month to '<i>DD</i>'.
    <b>-f</b> <i>FILE</i>         Change the timestamps to the modification time of '<i>FILE</i>'.
    <b>-m</b> <i>MM</i>           Change the month to '<i>MM</i>'.
    <b>-s</b>              Silent (not verbose) output.
    <b>-t</b> <i>TIME</i>         Change the timestamps to '<i>TIME</i>', which is of the format:
                        [<i>CC</i>]<i>YY</i>-<i>MM</i>-<i>DD</i>[.<i>hh</i>:<i>mm</i>[:<i>ss</i>[.<i>uuu</i>]]]
    <b>-u</b>              Timestamps are specified as UTC, not local time.
    <b>-v</b>              Verbose output (default).
    <b>-y</b> [<i>CC</i>]<i>YY</i>       Change the year to '<i>CCYY</i>'.
</pre>
If no date or <code>-f</code> option is specified, the current date and time will be used.

The <code>-f</code> option cannot be specified if the
<code>-t</code>, <code>-y</code>, <code>-m</code>, or <code>-d</code> options are also specified.

Filenames may contain wildcard characters (<code>?</code> and <code>*</code>).
