
The CRaTER Pipe2 Command
------------------------

SYNOPSIS

  pipe2 [-v] [-z] [-G n,n,n] [-d OutDir] 
        [-l LogDir] [-g GapDir] [-x IndexDir] [-L LGIDir]
        [-S] [-s SpiceFile] [-a AppidFile] [-h CoefFile] [-p PhaseFile] 
        [-P phase] [-n serial#] [-r rads] [-V version] 
        [-D DataDir] -y yyyyddd pathname [pathname ...]


COMPILING

  The CRaTER pipeline is designed to utilize the NAIF CSPICE Toolkit, available 
  here: http://naif.jpl.nasa.gov/naif/toolkit_C.html  

  This software has been built and tested using GNU Make and compilers (g++)
  on Linux systems.  Other utilities, compilers, or platforms may not be 
  compatible.

  After compiling CSPICE add 'SPICE=/path/to/cspice' to the make options, e.g.:
     `make clean all SPICE=../cspice`

  To build without SPICE functionality omit the SPICE variable, e.g.: 
     `make clean all`

  If SPICE is not provided the build script will still check the DEFAULT_SPICE 
  directory described in Makefile, and use that directory if it exists.

  A REVISION variable may also be provided when building for inclusion in the 
  output of processing logs.  REVISION defaults to the subversion revision 
  number if compiled within a subversion working directory, or the current 
  timestamp otherwise.


DESCRIPTION

  This command reads one or more raw CRaTER telemetry files containing some 
  mixture of the three types of data packet: primary science, secondary science,
  and housekeeping.  It indexes those data files for timestamps on a specified 
  day, and then indexes packet headers from the appropriate files.  Finally, the
  packets are processed and converted to Level 0, 1 and 2 products by 
  translating the raw counts in Level 0 primary science into electron volts and 
  LET.

  The processed output consists of (a) the data files (three each at Levels 0, 
  1, and 2), (b) their accompanying PDS label files, (c) a PDS 'index' file 
  describing the names and contents of the data files, (d) a file of data gaps, 
  and (e) a log file containing informatory messages.

  -a AppidFile  Read a description of how raw data values are read and formatted
                data written from AppidFile.  If this is unavailable, the 
                program will write an error message to the standard error 
                stream, cerr, and halt immediately.

  -D DataDir    Parent directory which contains yyyyddd sub-directories of data
                files.  Providing DataDir will automatically add 'pathname' 
                arguments for yyyyddd, as well as the day before and after.

  -d OutDir     All Level 0, 1 and 2 data files and their accompanying PDS label
                files will be written to the directory named OutDir.  If this 
                option is unspecified, the default is to write the files into
                the current directory.  If this directory either doesn't exist 
                or is unwritable, the program will write an error message to the
                standard error stream, cerr, and halt immediately.

  -G n,n,n      The minimum times, in seconds, between successive data packets 
                that will be reported as data gaps in primary science, secondary
                science, and housekeeping streams, respectively.  The default
                values are 2, 2, and 20.

  -g GapDir     The information about Level 0 data gaps will be written in a PDS
                approved format to a file named 'CRAT_yyyyddd_vnn_GAPS.TAB' in 
                the directory named GapDir, where yyyyddd and nn will be taken
                from the -y and -V options, respectively.  If the GapDir 
                directory doesn't exist, or is unwritable, the program will 
                write an error message to the standard error stream, cerr, and 
                halt.  If this option is omitted, the gap file will be written 
                into the current directory. 

  -h CoefFile   The pathname of a file containing CRaTER housekeeping conversion
                coefficients.  If the file is unavailable, the pipeline will 
                write a message to the standard error stream, cerr, and then 
                halt.

  -L LGIDir     Use LGIDir as the directory for logs, gaps, and index files.
                Equivalent to '-l LGIDir -g LGIDir -x LGIDir'.

  -l LogDir     The program will write informatory messages to a file in the 
                LogDir directory that will be named 'CRAT_yyyyddd_vnn_LOG.TXT', 
                where yyyyddd and nn will be taken from the -y and -V options, 
                respectively.  If the LogDir directory doesn't exist, or is 
                unwritable, the program will write an error message to the 
                standard error stream, cerr, and halt.  If this option is 
                omitted, the log file will be written into the current 
                directory. 

  -n serial#    Specify the instrument serial number.  If a serial number is not
                provided the program will write an error message to the 
                standard error stream, cerr, and halt.

  -P phase      Provides or expands 'MISSION_PHASE_NAME' from command line 
                arguments.  If unspecified, and phase name cannot be read 
                from the PhaseFile, the default value 'UNKNOWN' will be used.
                Multiple -P options may        be given.

  -p PhaseFile  Reads the 'MISSION_PHASE_NAME' keyword value in the PDS label 
                files that describes the LRO mission phase during which the 
                CRaTER data were recorded, e.g., 'EXPLORATION MISSION'.  
                PhaseFile is examined for an appropriate phase name for a given 
                yyyyddd.

  -r rads       Specify the cumulative radiation dosage prior to the last 
                instrument power-up.  The default prior dosage is 0.

  -S            Flag to disable the use of SPICE, if the pipeline has been built
                to use the SPICE library.  NOTE: Level 2 output will be
                incomplete if SPICE is disabled.

  -s file       Specify a SPICE kernel file.  Several kernels are required for
                successful operation: SPK describing the spacecraft ephemeris;
                SPK describing Earth, Moon, and Sun; CK describing the 
                orientation of the LRO spacecraft; SCLK describing the LRO 
                spacecraft clock; FK describing the location and orientation of
                CRaTER in LRO spacecraft coordinates; PCK defining planetary 
                constants; and LSK defining leap seconds.  These can be supplied
                by multiple -s options, or all together in the value of the 
                'KERNELS_TO_LOAD' keyword in a single 'meta-kernel' file. 
                Consult the SPICE cookbook at 'http://naif.jpl.nasa.gov/naif'
                for details.

  -V version    Specify the version number of the products to be created by this
                program.  Version must be an integer between 0 and 99.  The 
                default value is 1.

  -v            Increase the quantity and level of detail in the messages to be 
                written to the log file (see -l, above).

  -x IndexDir   PDS index information about Level 0, 1 and 2 data files will be 
                written to a file named 'CRAT_yyyyddd_vnn_INDEX.TAB' in the 
                directory named IndexDir, where yyyyddd and nn will be taken 
                from the -y and -V options, respectively.  If the IndexDir 
                directory doesn't exist, or is unwritable, the program will 
                write an error message to the standard error stream, cerr, and 
                halt.  If this option is omitted, the index file will be written
                into the current directory. 

  -y yyyyddd    Specify the year and day of year of the data packets to be 
                extracted from the input EDR file(s).  Only one 24-hour interval
                can be processed at a time.

  -z            Compress the Level 1 and 2 output files as they are being 
                written.  A gzip(1) process is forked for each of the 6 output 
                data files.

  pathname ...  One or more pathnames of CRaTER raw data files, or of 
                directories that contain other directories and/or raw CRaTER 
                files.  Nested directories will be searched to any depth, but 
                infinite loops, e.g., those constructed with symbolic links, 
                are not detected.


FILES

  INPUT

    The following configuration files are used in processing Level 0 data:

    conf/crat_hk_table_v02.txt

        A table of CRaTER housekeeping conversion coefficients that is read in 
        order to translate Level 0 housekeeping channels into their Level 1 and
        2 values.

    conf/appids.txt

        Information on how raw data values are read and formatted data written.

    conf/phases.txt

        Start and stop dates for mission phases.

  OUTPUT

    In the following list, output file names inherit their yyyyddd and nn 
    fields from the values of the -y and -V flags, respectively:

    CRAT_yyyyddd_vnn_GAPS.TAB

        Output gap file in PDS format to be written to the directory specified 
        by the -g flag.  Except where noted, each field is in uppercase 
        characters, and is left justified, blank filled, and enclosed in double 
        quotes.  The fields are comma delimited, and each line is terminated by 
        a carriage-return followed by a line-feed:

            * File name (31 characters)
            * Date of last packet before the gap (UTC as yyyy-mm-ddThh:mm:ss)
            * Date of first packet after the gap (UTC as yyyy-mm-ddThh:mm:ss)
            * S/C clock time of last packet before the gap (nnnnnnnnnn:nn)
            * S/C clock time of first packet after the gap (nnnnnnnnnn:nn)
            * Gap duration (in seconds, 10 bytes, right justified, no quotes)

    CRAT_yyyyddd_vnn_INDEX.TAB

        Output index file in PDS format to be written to the directory specified
        by the -x flag.  Each field is in uppercase, and is left justified, 
        blank filled, and enclosed in double quotes.  The fields are comma 
        delimited, and each line is terminated by a carriage-return followed by 
        a line-feed:

            * File name (31 characters)
            * Path name (50 characters)
            * Date of first packet (UTC as yyyy-mm-ddThh:mm:ss.sss)
            * Date of last packet (UTC as yyyy-mm-ddThh:mm:ss.sss)
            * PDS Product (12 characters, e.g., CRAT_L0_PRI)
            * PDS Dataset (40 characters, e.g., LRO-CRAT-2-EDR-RAWDATA-V1.0)
            * Production date (yy-mm-dd)

    CRAT_yyyyddd_vnn_LOG.TXT

        Output log file in ASCII format to be written to the directory specified 
        by the -l flag.  A sample log file might contain the following...

        ---

        PDS_VERSION_ID     = PDS3
        RECORD_TYPE        = STREAM
        OBJECT             = TEXT
        NOTE               = "Pipeline log file."
        PUBLICATION_DATE   = 2010-03-09
        END_OBJECT         = TEXT
        END
        
        pipe2 starting on 2010-03-09T19:01:13
        
        Year and day-of-year:        2009180 (2009-06-29)
        Product version number:      1
        Mission phase:               "COMMISSIONING"
        Prior radiation dosage:      0
        Output data file directory:  /soc_work/pds/processed/2009180
        Output log file:             /soc_work/pds/logs/2009180/CRAT_2009180_V01_LOG.TXT
        Output gap file:             /soc_work/pds/logs/2009180/CRAT_2009180_V01_GAPS.TAB
        Output index file:           /soc_work/pds/logs/2009180/CRAT_2009180_V01_INDEX.TAB
        Appid definitions file:      /usr/local/crater/bin/conf/appids.txt
        Phase definitions file:      /usr/local/crater/bin/conf/phases.txt
        Housekeeping parameter file: /usr/local/crater/bin/conf/crat_hk_table_v02.txt
        Minimum gap times:           2 2 20
        Instrument serial number:    2
        Verbose mode:                1
        Compression mode (Lvl 1,2):  on
        Using SPICE:                 yes
        Machine byte order:          little
        Pipeline build revision:     302
        
        Furnished spice file /working/pds/spice/kernels.txt
        Found data file /working/pds/data/2009179/CRAT_2009179_0000540.sci 0267848114.75 (2009179) to 0267849789.75 (2009179)
        Found data file /working/pds/data/2009179/CRAT_2009179_0000587.sci 0267918755.75 (2009179) to 0267918876.00 (2009179)
        Found data file /working/pds/data/2009179/CRAT_SC_2009179_0001227.hk 0267855769.93 (2009179) to 0267856424.62 (2009179)
        .
        .
        .
        Ready to index: 0.1s 0.0s 0:03 5% 0+0k 0+0io 0pf+0s 0ss+0m+1468r

        /working/pds/data/2009173/CRAT_2009173_0000225.hk: read 1262 records, 630 accepted.
        /working/pds/data/2009173/CRAT_2009173_0000225.sci: read 2858 records, 1443 accepted.
        /working/pds/data/2009173/CRAT_SC_2009173_0000537.hk: read 1463 records, 198 accepted.
        .
        .
        .
        Indexed files: 3.1s 6.3s 0:32 29% 0+0k 0+0io 0pf+0s 0ss+0m+8523r

        Processed files: 355.3s 7.4s 6:29 93% 0+0k 0+0io 1pf+0s 0ss+0m+3183r

        PRI : 193924 packets processed
        SEC : 82396 packets processed
        HK : 5149 packets processed

        Ending radiation dose: 0.02212 rads 

        Finished pipe2 on 2010-02-09T18:27:45                                                        

    CRAT_L0_type_yyyyddd_vnn.LBL
    CRAT_L1_type_yyyyddd_vnn.LBL
    CRAT_L2_type_yyyyddd_vnn.LBL

        PDS detached label files describing the nine output data files of the 
        same name with extension 'DAT' replaced by 'LBL'.  They will be written
        into the same directory as the output data files.

SEE ALSO
    
    'CRaTER Calibration Plan', 32-01207, Revision A-, September 4, 2007

    'CRaTER Standard Product Data Record and Archive Volume Software Interface 
    Specification', 32-01211, Revision B, August 1, 2007.


LICENSE

    The MIT License

    Copyright (c) 2010 Erik Wilson, Peter Ford

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use,
    copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following
    conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.

