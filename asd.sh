REDIRECT_STATUS=true
SCRIPT_FILENAME=test.php
REQUEST_METHOD=POST
GATEWAY_INTERFACE=CGI/1.1
CONTENT_TYPE=application/x-www-form-urlencoded;
QUERY_STRING="hoi=34&kjfdsgf=3"
CONTENT_LENGTH=7
# QUERY_STRING="asd=34&kjasd=4"
export REDIRECT_STATUS
export SCRIPT_FILENAME
export REQUEST_METHOD
export GATEWAY_INTERFACE
export CONTENT_TYPE
export QUERY_STRING
export CONTENT_LENGTH
# export QUERY_STRING
echo "test=\"1" | php-cgi


# echo "t=1" | 
# GATEWAY_INTERFACE="CGI/1.1"
# SCRIPT_FILENAME="~/MyPrograms/TCPtest/test.php"
# REQUEST_METHOD="POST"
# REDIRECT_STATUS=200
# SERVER_PROTOCOL="HTTP/1.1"
# REMOTE_HOST="127.0.0.1"
# CONTENT_LENGHT=3
# HTTP_ACCEPT="text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8"
# CONTENT_TYPE="application/x-www-form-urlencoded"
# BODY="t=1" php-cgi
