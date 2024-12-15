#!/usr/bin/env bash

function bpp_Object_new() {
	local objectName="$1"
	eval "bpp_Object_${objectName}_data=''"
}

bpp_Object_testObject1_data="default value for primitive data member"
bpp_Object_testObject3_data="default value for primitive data member"
bpp_Object_testObject4_data="$bpp_Object_testObject3_data"

echo "$bpp_Object_testObject1_data"
echo ""

bpp_Object_testObject3_data="new value for object number 3"
echo "$bpp_Object_testObject3_data"
echo "$bpp_Object_testObject3_data"

echo "$bpp_Object_testObject4_data"
echo "$bpp_Object_testObject4_data"

unset bpp_Object_testObject1_data
unset bpp_Object_testObject1_data
unset bpp_Object_testObject3_data
unset bpp_Object_testObject3_data
unset bpp_Object_testObject4_data
unset bpp_Object_testObject4_data

echo "$bpp_Object_testObject1_data"

