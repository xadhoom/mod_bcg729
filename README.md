mod_bcg729
==========

FreeSWITCH G.729A module using the opensource bcg729 implementation by Belledonne Communications.

Simple G.729A codec for FreeSWITCH using the Belledone Communications G.729A GPLv2 implementation.
Please see http://www.linphone.org/eng/documentation/dev/bcg729.html for further informations.

This code is a modified version of fsg729 ( https://code.google.com/p/fsg729/ ) which
uses the Intel IPP libraries, modified to use a different code and get rid of Intel stuff.

To use G.729 or G.723.1 you may need to pay a royalty fee.
Please see http://www.sipro.com for details.
Please note that this code is available for you to download for education purposes 
only and if a patent exists in your country for G.729 then you should contact 
the owner of that patent and request their permission before executing the code.

You can get a licensed, faster and supported G.729A codec by purchasing licenses
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
