<?php
$target_dir = "uploads/";
$target_file = $target_dir . basename($_FILES["fileToUpload"]["name"]);
$uploadOk = 1;
$imageFileType = strtolower(pathinfo($target_file,PATHINFO_EXTENSION));

// Check if image file is a actual image or fake image
if(isset($_POST["submit"])) {
    $check = getimagesize($_FILES["fileToUpload"]["tmp_name"]);
    if($check !== false) {
        echo "파일은 이미지입니다. - " . $check["mime"] . ".";
        $uploadOk = 1;
    } else {
        echo "파일은 이미지가 아닙니다.";
        $uploadOk = 0;
    }
}

// Check if file already exists
if (file_exists($target_file)) {
    echo "죄송합니다, 파일이 이미 존재합니다.";
    $uploadOk = 0;
}

// Check file size
if ($_FILES["fileToUpload"]["size"] > 500000) {
    echo "죄송합니다, 파일이 너무 큽니다.";
    $uploadOk = 0;
}

// Allow certain file formats
if($imageFileType != "jpg" && $imageFileType != "png" && $imageFileType != "jpeg"
&& $imageFileType != "gif" ) {
    echo "죄송합니다, 오직 JPG, JPEG, PNG & GIF 파일만 허용됩니다.";
    $uploadOk = 0;
}

// Check if $uploadOk is set to 0 by an error
if ($uploadOk == 0) {
    echo "죄송합니다, 파일이 업로드되지 않았습니다.";
// if everything is ok, try to upload file
} else {
    if (move_uploaded_file($_FILES["fileToUpload"]["tmp_name"], $target_file)) {
        echo "파일 ". basename( $_FILES["fileToUpload"]["name"]). " 가 업로드되었습니다.";
    } else {
        echo "죄송합니다, 파일 업로드 중 오류가 발생했습니다.";
    }
}
?>
