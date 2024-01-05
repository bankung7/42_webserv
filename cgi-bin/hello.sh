#!/bin/bash

echo "Content-type: text/html"
echo ""

echo '<html>'
echo '<head>'
# echo '<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">'
echo '<title>Bash CGI Script</title>'
echo '</head>'
echo '<body>'
# echo 'Parse Variables:'

# Save the old internal field separator.
  OIFS="$IFS"

# Set the field separator to & and parse the QUERY_STRING at the ampersand.
  IFS="${IFS}&"
  set $QUERY_STRING
  Args="$*"
  IFS="$OIFS"

# Next parse the individual "name=value" tokens.

  firstName=""
  lastName=""

  for i in $Args ;do

#       Set the field separator to =
        IFS="${OIFS}="
        set $i
        IFS="${OIFS}"

        case $1 in
                # Don't allow "/" changed to " ". Prevent hacker problems.
                firstname) firstName="`echo $2 | sed 's|[\]||g' | sed 's|%20| |g'`"
                       ;;
                # Filter for "/" not applied here
                lastname) lastName="`echo $2 | sed 's|%20| |g'`"
                       ;;
                # *)     echo "<hr>Warning:"\
                #             "<br>Unrecognized variable \'$1\' passed by FORM in QUERY_STRING.<hr>"
                #        ;;

        esac
  done

echo '<h1>Hello, '$lastName $firstName'</h1>'
echo '<p>This is a Bash CGI Script</p>'

echo '</body>'
echo '</html>'
