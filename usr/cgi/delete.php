#!/usr/bin/php
<?php
$method = getenv('REQUEST_METHOD');
if ($method == 'delete' || $method == 'post') {
    parse_str(file_get_contents("php://input"), $_DELETE);
    $file = $_DELETE['file'];
    $dir = './usr/upload'; // 디렉토리 경로를 설정하세요.
    $file_path = $dir . DIRECTORY_SEPARATOR . $file;

    if (file_exists($file_path)) {
        if (unlink($file_path)) {
            echo "File deleted successfully.";
        } else {
            http_response_code(500);
            echo "Error deleting file.";
        }
    } else {
        http_response_code(404);
        echo "File does not exist.";
    }
} else {
    http_response_code(405);
    echo "Invalid request method.";
}
?>