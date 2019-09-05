#!/bin/bash


#wuyujie

THIRD_LIB_DIR=3rd
JSON=json-c-0.9
JSON_DIR=${THIRD_LIB_DIR}/json/${JSON}

PWD_DIR=$('pwd')
echo ${PWD}

if [ ! -d 3rd ];  then 
    echo "not found 3rd party libraries"
    exit -1 
else
    echo "found 3rd libraries directory!"
fi

####################build json###########################
cd ${JSON_DIR}
chmod +x configure

if [ ! -d ${PWD_DIR}/src/include/libobject/include ]; then 
    rm -rf ${PWD_DIR}/src/include/libobject/include     
fi 

if [ ! -d ${PWD_DIR}/src/include/libobject/lib ]; then 
    rm -rf ${PWD_DIR}/src/include/libobject/lib 
fi 

./configure  --prefix=${PWD_DIR}/src/include/libobject --disable-static 
make && make install 

if [ $? -ne 0 ]; then
    echo "json-c compile  failed"
    exit -1
else
    ln -sf    ${PWD_DIR}/src/include/libobject/include/json/arraylist.h           ${PWD_DIR}/src/include/libobject/json/arraylist.h            
    ln -sf    ${PWD_DIR}/src/include/libobject/include/json/json_object.h         ${PWD_DIR}/src/include/libobject/json/json_object.h           
    ln -sf    ${PWD_DIR}/src/include/libobject/include/json/linkhash.h            ${PWD_DIR}/src/include/libobject/json/linkhash.h             
    ln -sf    ${PWD_DIR}/src/include/libobject/include/json/json_object_private.h ${PWD_DIR}/src/include/libobject/json/json_object_private.h  
    ln -sf    ${PWD_DIR}/src/include/libobject/include/json/printbuf.h            ${PWD_DIR}/src/include/libobject/json/printbuf.h             
    ln -sf    ${PWD_DIR}/src/include/libobject/include/json/json_tokener.h        ${PWD_DIR}/src/include/libobject/json/json_tokener.h         
    ln -sf    ${PWD_DIR}/src/include/libobject/include/json/json_util.h           ${PWD_DIR}/src/include/libobject/json/json_util.h            
    ln -sf    ${PWD_DIR}/src/include/libobject/include/json/debug.h               ${PWD_DIR}/src/include/libobject/json/debug.h                
    ln -sf    ${PWD_DIR}/src/include/libobject/include/json/json.h                ${PWD_DIR}/src/include/libobject/json/json.h                 
    ln -sf    ${PWD_DIR}/src/include/libobject/include/json/bits.h                ${PWD_DIR}/src/include/libobject/json/bits.h                 

    mv ${PWD_DIR}/src/include/libobject/lib/libjson.*          ${PWD_DIR}/lib/linux
    rm -rf   ${PWD_DIR}/src/include/libobject/lib
fi
##############json build end##################################



