
LAST_IMAGE=$(docker image ls | head -n 2 | tail -n 1 | awk '{ print $3 }')
CONTAINER=$(docker create ${LAST_IMAGE})
mkdir -p bin
docker cp ${CONTAINER}:/home/zetasql/bin/execute_query ./bin/execute_query_linux_amd64 
chown ${USER} execute_query_linux_amd64
chown ${USER} execute_query_linux_amd64