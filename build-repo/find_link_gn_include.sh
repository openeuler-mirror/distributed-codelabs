#!/bin/bash
current_path=$(cd $(dirname $0); pwd)
link_type=""
gn_or_include_old_path=""
gn_or_include_new_path=""
copyt_module_path=""
include_link_path=""

function log_info(){
    message=$1
    echo "[INFO] ${message}"
}

function log_error(){
    message=$1
    echo "[ERROR] ${message}"
}

function check_path_exist(){
    check_path=$1
    if [[ ! -d ${check_path} ]];then
        log_error "${check_path} not found, please check"
        exit 1
    fi
}

function find_and_copy_gn(){
    log_info "Start find and copy BUIL.gn from ${gn_or_include_old_path} to ${gn_or_include_new_path}"
    check_path_exist ${gn_or_include_old_path}
    check_path_exist ${gn_or_include_new_path}
    cd ${gn_or_include_old_path}
    gn_files=$(find ${copyt_module_path} -name 'BUILD.gn')
    for gn_file in ${gn_files};do
        /bin/cp --parent ${gn_file} ${gn_or_include_new_path}
        log_info "Copy ${gn_file} end"
    done
    log_info "Find and copy BUIL.gn from ${gn_or_include_old_path} to ${gn_or_include_new_path} End"
}

find_and_link_include(){
    log_info "Start find and link include from ${gn_or_include_old_path} to ${gn_or_include_new_path}"
    check_path_exist ${gn_or_include_old_path}
    check_path_exist ${gn_or_include_new_path}

    cd ${gn_or_include_old_path}
    include_pathes=$(find ${gn_or_include_old_path} -name "include" -type d)
    for include_path in ${include_pathes};do
        dirname_include_path=${include_path%include*}
        mkdir -p ${gn_or_include_new_path}/${dirname_include_path}
        if [[ -n ${include_link_path} ]];then
            check_path_exist ${include_link_path}
            ln -s ${include_link_path} ${gn_or_include_new_path}/${dirname_include_path}/include
            log_info "Copy and link ${include_path} end"
        else
            log_info "Copy ${include_path} end"
        fi
    done

    log_info "Find and link include from ${gn_or_include_source} to ${gn_or_include_new_path} end"
}

function help_info(){
cat << EOF
****************************************
bash find_link_gn_include.sh -t|--type gn/include -od|--old-path  旧编译路径 -nd|--new-path 新编译路径 -m|--module 拷贝的模块路径  -il|--include-link 要link的include目录
参数说明：
-t|--type  拷贝内容的类型，gn或者include
-od|--old-path  拷贝内容的源路径
-nd|--new-path  拷贝内容的目标路径（新编译根目录）
-m|--module  拷贝内容所在的模块路径，基于源路径的相对路径
-il|--include-link  要link的头文件路径
****************************************
EOF
}

function main(){
    while [ -n "$1" ]
        do
            case $1 in 
                -t|--type)
                    link_type=$2
                    shift 2
                    ;;
                -od|--old-path)
                    gn_or_include_old_path=$2
                    shift 2
                    ;;
                -nd|--new-path)
                    gn_or_include_new_path=$2
                    shift 2
                    ;;
                -m|--module)
                    copyt_module_path=$2
                    shift 2
                    ;;
                -il|--include-link)
                    include_link_path=$2
                    shift 2
                    ;;
                *)
                    help_info
                    exit 1
                    ;;
            esac
        done
    if [[ ${link_type} == "gn" ]];then
        find_and_copy_gn
    elif [[ ${link_type} == "include" ]];then
        find_and_link_include
    else 
        log_error "The value of parameter '-t' is not supported"
        exit 1
    fi
}

main $@
