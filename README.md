The [CRaTER](http://crater.sr.unh.edu/) 'pipe2'
  command reads one or more raw CRaTER telemetry files containing some 
  mixture of the three types of data packet: primary science, secondary science,
  and housekeeping.  It indexes those data files for timestamps on a specified 
  day, and then indexes packet headers from the appropriate files.  Finally, the
  packets are processed and converted to Level 0, 1 and 2 products by 
  translating the raw counts in Level 0 primary science into electron volts and 
  LET.
  
Original version can be found at the
[PDS archives](http://pds-ppi.igpp.ucla.edu/search/view/?f=yes&id=pds://PPI/LROCRA_2XXX/SOFTWARE/PIPE2&o=1).
