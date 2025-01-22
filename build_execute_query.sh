#bazel --output_user_root ./.bazel_cache build fetch -c opt //zetasql/tools/execute_query:execute_query
#bash ./fix_lsp_packages.sh ./.bazel_cache build
bazel --output_user_root ./.bazel_cache build --nofetch -c opt --dynamic_mode=off //zetasql/tools/execute_query:execute_query
cp ./bazel-bin/zetasql/tools/execute_query/execute_query ./bin/execute_query_linux_amd64