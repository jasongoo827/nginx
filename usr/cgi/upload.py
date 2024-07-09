#!/usr/bin/env python3

import os
import cgi
import cgitb
import shutil

cgitb.enable()

UPLOAD_DIR = "uploads/"

def is_image(file_path):
    from PIL import Image
    try:
        with Image.open(file_path) as img:
            return True
    except IOError:
        return False

def main():
    print("Content-Type: text/html")    # HTML is following
    print()                             # blank line, end of headers

    form = cgi.FieldStorage()
    if "fileToUpload" not in form:
        print("<h1>No file was uploaded</h1>")
        return

    file_item = form["fileToUpload"]

    # Check if the file was uploaded
    if not file_item.file:
        print("<h1>No file was uploaded</h1>")
        return

    # Save the uploaded file
    file_path = os.path.join(UPLOAD_DIR, os.path.basename(file_item.filename))
    
    # Check if file is an actual image
    if not is_image(file_item.file):
        print("<h1>File is not an image</h1>")
        return

    # Check if file already exists
    if os.path.exists(file_path):
        print("<h1>File already exists</h1>")
        return

    # Check file size (example limit: 500000 bytes)
    file_item.file.seek(0, os.SEEK_END)
    file_size = file_item.file.tell()
    file_item.file.seek(0, os.SEEK_SET)
    if file_size > 500000:
        print("<h1>File is too big</h1>")
        return

    # Save the file
    with open(file_path, 'wb') as output_file:
        shutil.copyfileobj(file_item.file, output_file)

    print(f"<h1>File {os.path.basename(file_item.filename)} is uploaded.</h1>")

if __name__ == "__main__":
    main()