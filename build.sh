#!/bin/bash

while getopts ":s" opt; do
  case $opt in
    s)
	  SILENT=true
	  command shift
      ;;
  esac
done
echo "Building OpenHome..."

if [ "$1" == "demo" ]; then
	g++ -o OpenHome -Wno-int-to-pointer-cast -DDEMO main.cpp OpenHome.cpp program.cpp server.cpp utils.cpp weather.cpp gpio.cpp etherport.cpp -lpthread
else
	g++ -o OpenHome -Wno-int-to-pointer-cast -DOSPI -DPINE main.cpp OpenHome.cpp program.cpp server.cpp utils.cpp weather.cpp gpio.cpp etherport.cpp -lpthread
fi

# if [ ! "$SILENT" = true ] && [ -f OpenHome.launch ] && [ ! -f /etc/init.d/OpenHome.sh ]; then

# 	read -p "Do you want to start OpenHome on startup? " -n 1 -r
# 	echo

# 	if [[ ! $REPLY =~ ^[Yy]$ ]]; then
# 		exit 0
# 	fi

# 	echo "Adding OpenHome launch script..."

# 	# Get current directory (binary location)
# 	pushd `dirname $0` > /dev/null
# 	DIR=`pwd`
# 	popd > /dev/null

# 	# Update binary location in start up script
# 	sed -e 's,\_\_OpenHome\_Path\_\_,'"$DIR"',g' OpenHome.launch > OpenHome.sh

# 	# Make file executable
# 	chmod +x OpenHome.sh

# 	# Move start up script to init.d directory
# 	sudo mv OpenHome.sh /etc/init.d/

# 	# Add to auto-launch on system startup
# 	sudo update-rc.d OpenHome.sh defaults

# 	# Start the deamon now
# 	sudo /etc/init.d/OpenHome.sh start

# fi

echo "Done!"
