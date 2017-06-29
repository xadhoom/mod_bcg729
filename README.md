mod_bcg729
==========

FreeSWITCH G.729A module using the opensource bcg729 implementation by Belledonne Communications.

Simple G.729A codec for FreeSWITCH using the Belledonne Communications G.729A GPLv2 implementation.
Please see http://www.linphone.org/eng/documentation/dev/bcg729.html for further informations.

The module is a modified version of fsg729 ( https://code.google.com/p/fsg729/ ) which
uses the Intel IPP libraries, updated to use a different codec implementation and get rid of Intel stuff.

As of Jan 1 2017, G.729 is a royalty free codec: http://www.sipro.com/G729.html

You can get a faster and supported G.729A codec by purchasing licenses
directly from FreeSWITCH guys http://www.freeswitch.org .
This will have the side effect to support the FreeSWITCH project ;)

Installation
============
You need to have git on your build machine and internet access, since
the Makefile will try to checkout bcg729 sources and build them.

Edit Makefile and edit FS_INCLUDES, FS_MODULES vars to point where
your FreeSWITCH includes are and where you want to install the module.

After, just type make and, if build completes without errors, make install .

Edit autoload_configs/modules.conf.xml , comment out mod_g729 and add mod_bcg729 .
Now restart your FreeSWITCH and you're done.
