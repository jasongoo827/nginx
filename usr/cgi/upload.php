#!/usr/bin/php
<?php
$target_dir = "uploads/";
$target_file = $target_dir . basename($_FILES["fileToUpload"]["name"]);
$uploadOk = 1;
$imageFileType = strtolower(pathinfo($target_file,PATHINFO_EXTENSION));

// Check if image file is a actual image or fake image
if(isset($_POST["submit"])) {
    $check = getimagesize($_FILES["fileToUpload"]["tmp_name"]);
	file_put_contents('post_data.log', print_r($_POST, true));
	file_put_contents('files_data.log', print_r($_FILES, true));
    if($check !== false) {
        echo "file is image. - " . $check["mime"] . ".";
        $uploadOk = 1;
    } else {
        echo "file is not image";
        $uploadOk = 0;
    }
}

// Check if file already exists
if (file_exists($target_file)) {
    echo "file already exist";
    $uploadOk = 0;
}

// Check file size
if ($_FILES["fileToUpload"]["size"] > 500000) {
    echo "file is too big.";
    $uploadOk = 0;
}

// Allow certain file formats
// if($imageFileType != "jpg" && $imageFileType != "png" && $imageFileType != "jpeg"
// && $imageFileType != "gif" ) {
//     echo "only JPG, JPEG, PNG & GIF file allowed.";
//     $uploadOk = 0;
// }

// Check if $uploadOk is set to 0 by an error
if ($uploadOk == 0) {
    echo "file is not uploaded.";
// if everything is ok, try to upload file
} else {
    if (move_uploaded_file($_FILES["fileToUpload"]["tmp_name"], $target_file)) {
        echo "file ". basename( $_FILES["fileToUpload"]["name"]). " is uploaded.";
    } else {
        echo "error during file upload.";
    }
}
?>
