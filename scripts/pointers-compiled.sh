#!/usr/bin/env bash

function bpp_Object__new() {
	local objectName="$1"
	eval "bpp_Object_${objectName}_data='default value for primitive data member'"
}

function bpp_Object__copy() {
	local originalObjectName="$1" newObjectName="$2"
	local originalObject_dataVar="bpp_Object_${originalObjectName}_data"
	local newObject_dataVar="bpp_Object_${newObjectName}_data"
	local originalObject_data="${!originalObject_dataVar}"
	eval "$newObject_dataVar='$originalObject_data'"
}

function bpp_Object__delete() {
	local objectName="$1"
	eval "unset bpp_Object_${objectName}_data"
}

bpp_Object__new "testObject1"
bpp__ptr_Object_testObject2="nullptr"
bpp_Object__new "testObject3"
bpp__ptr_Object_testObject3="testObject3"
bpp__ptr_Object_testObject3Copy="testObject3"
bpp_Object__copy "$bpp__ptr_Object_testObject3" "testObject4"
bpp__ptr_Object_testObject5="testObject4"

echo "$bpp_Object_testObject1_data"
bpp__ptr_Object_testObject2_data="bpp_Object_${bpp__ptr_Object_testObject2}_data"
echo "${!bpp__ptr_Object_testObject2_data}"

bpp__ptr_Object_testObject3_data="bpp_Object_${bpp__ptr_Object_testObject3}_data"
eval "$bpp__ptr_Object_testObject3_data='new value for object number 3'"
echo "${!bpp__ptr_Object_testObject3_data}"
bpp__ptr_Object_testObject3Copy_data="bpp_Object_${bpp__ptr_Object_testObject3Copy}_data"
echo "${!bpp__ptr_Object_testObject3Copy_data}"

echo "$bpp_Object_testObject4_data"
bpp__ptr_Object_testObject5_data="bpp_Object_${bpp__ptr_Object_testObject5}_data"
echo "${!bpp__ptr_Object_testObject5_data}"

bpp_Object__delete "testObject1"
bpp_Object__delete "$bpp__ptr_Object_testObject2"
bpp_Object__delete "$bpp__ptr_Object_testObject3"
bpp_Object__delete "$bpp__ptr_Object_testObject3Copy"
bpp_Object__delete "testObject4"
bpp_Object__delete "$bpp__ptr_Object_testObject5"

echo "$bpp_Object_testObject1_data"

