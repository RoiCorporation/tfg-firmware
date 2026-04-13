#!/bin/bash

set -e  # Stop if any command fails.

# Paths to the relevant folders and files.
PNG_FOLDER="media/png"
BMP_FOLDER="media/bmp"
ICONS_HEADER_FILES_FOLDER="include/icons_header_files"
INCLUDE_FOLDER="include"
BIN2C_PROGRAM_FOLDER="lib/pico-ssd1306/tools"
ICONS_HEADER_FILE="include/icons.h"


# Only run the script if the user is in the firmware's root folder.
current_dir=$(pwd)
if [ "${current_dir##*/}" != "central_station_firmware" ]; then
    echo "Can't run this script from outside of the central_station_firmware\
 root directory."
    exit
else

    # Create the dependent folders if they don't exist yet.
    mkdir -p $INCLUDE_FOLDER $BMP_FOLDER $ICONS_HEADER_FILES_FOLDER

    # Create and initialize the icons.h file if it doesn't exist yet.
    echo -e "#ifndef ICONS_H\n#define ICONS_H\n\n" > \
    $ICONS_HEADER_FILE

    # Compile ssd1306 library's own bin2c program that is used to generate each
    # icon's bytemap and save it into its own header file.
    make -C $BIN2C_PROGRAM_FOLDER

    # Iterate through each PNG file in the PNG folder and generate and dump its
    # bitmap into the BMP folder.
    for icon_png_file in $(ls $PNG_FOLDER)
    do
        icon_name=${icon_png_file%.*}

        # Generate the bitmap (.bmp) file for each icon and save it inside the
        # BMP files folder.
        magick "$PNG_FOLDER/$icon_png_file" -background white -alpha remove \
        -alpha off -colors 2 -type bilevel BMP3:"$BMP_FOLDER/$icon_name.bmp"

        # Generate and write the bytemap representation of each icon in a header
        # file and store it in the appropriate folder.
        ./$BIN2C_PROGRAM_FOLDER/bin2c "$BMP_FOLDER/$icon_name.bmp" \
        "$ICONS_HEADER_FILES_FOLDER/$icon_name.h"

        # Include the generated icon header file in the icons.h file.
        echo -e "#include \"${ICONS_HEADER_FILES_FOLDER#*/}/$icon_name.h\"" >> \
        $ICONS_HEADER_FILE
        
    done

    echo -e "\n\n#endif" >> $ICONS_HEADER_FILE

fi

echo "✅ All bytemap header files created"
