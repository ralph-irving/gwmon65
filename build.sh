#!/bin/bash
IMG=/opt/mess/disks/apple/serialmon.po
VOL=/PROFILES
CFG=apple2enh-asm.cfg

if [ -f serialmon ]; then
	rm serialmon
fi

if [ -f apple2.o ]; then
	rm apple2.o
fi

if [ -f crtsetup1 ]; then
	rm crtsetup1
fi

ca65 -t apple2 -l apple2.lst -o apple2.o apple2.a65
cl65 -t apple2 -C $CFG -o serialmon apple2.o

ca65 -t apple2 -l crtsetup1.lst -o crtsetup1.o crtsetup1.s
cl65 -t apple2 -C $CFG -o crtsetup1 crtsetup1.o

srec_cat crtsetup1 -binary -offset 0x0300 -o crtsetup1.hex -intel -address-length=2 -esa 0x300

package () {
    IMGFILE="$1"
    IMGSIZE="$2"

    PACKDIR=$(mktemp -d)
    VOLNAME="PROFILES"

    rm -f "$IMG"
    cp -p ${HOME}/source/apple/80ColumnCard/software/blankpd243.po $IMG
    # cadius CREATEVOLUME "$IMGFILE" "$VOLNAME" "$IMGSIZE" --no-case-bits --quiet

    add_file () {
        cp "$1" "$PACKDIR/$2"
        cadius ADDFILE "$IMGFILE" "/$VOLNAME" "$PACKDIR/$2" --no-case-bits --quiet
    }

    add_file "./serialmon" "serialmon#069000"
    add_file "./crtsetup1" "crtcsetup1#060300"
    # add_file "${HOME}/source/apple2intbasic/out/intbasic.system.SYS" "IntBASIC.system#FF0000"
    # add_file "res/PRODOS.SYS" "PRODOS#FF0000"
    # add_file "${HOME}/source/apple2intbasic/res/WOZ.BREAKOUT.INT" "WOZ.BREAKOUT#FA0000"
    # add_file "${HOME}/source/apple2intbasic/res/APPLEVISION.INT" "APPLEVISION#FA0000"
    # add_file "out/readme" "README#040000"

    rm -r "$PACKDIR"

    cadius CATALOG $IMG
}

package "$IMG"  "140KB"
#package "out/intbasic_system.2mg" "800KB"

cat crtsetup1.hex
