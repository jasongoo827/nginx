#!/usr/bin/php
<?php
$method = getenv('REQUEST_METHOD');
if ($method == 'DELETE') {
    $input = json_decode(file_get_contents("php://input"), true);
    $file = $input['file'];
    $dir = getenv('PATH_FILEUPLOAD'); // 디렉토리 경로를 설정하세요.
    $file_path = $dir . DIRECTORY_SEPARATOR . $file;
    header('Content-Type: application/json');
    if (file_exists($file_path)) {
		chmod($file_path, 0777);
        if (unlink($file_path)) {
            echo json_encode(["message" => "File deleted successfully."]);
        } else {
            http_response_code(500);
            echo json_encode(["message" => "Error deleting file."]);
        }
    } else {
        http_response_code(404);
        echo json_encode(["message" => "File does not exist."]);
    }
} else {
    http_response_code(405);
    echo json_encode(["message" => "Invalid request method."]);
}
?>