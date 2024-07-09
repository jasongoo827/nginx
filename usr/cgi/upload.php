#!/usr/bin/php
<?php
// 환경 변수 사용
$contentType = getenv('CONTENT_TYPE');
$contentLength = getenv('CONTENT_LENGTH');
$result = "";
// 표준 입력에서 본문 데이터 읽기
$stdin = fopen('php://stdin', 'r');
if ($stdin == false)
	$result .= "stdin open error";
$requestBody = '';

if ($contentLength > 0)
{
	// Content-Length 만큼 반복하여 읽기
	while ($contentLength > 0)
	{
		// 읽을 바이트 수 결정
		$readBytes = min(8192, $contentLength); // 한 번에 최대 8192 바이트 읽기
		$data = fread($stdin, $readBytes);

        if ($data === false)
		{
			break; // 읽기 실패 시 종료
		}
		$requestBody .= $data;
		$contentLength -= strlen($data); // 남은 바이트 수 감소
	}
}

if ($data == false)
	$result .= "read failed";

preg_match('/boundary=(.*)$/', $_SERVER['CONTENT_TYPE'], $matches);
$boundary = "--" . $matches[1];

$parts = explode($boundary, $requestBody);

array_pop($parts);

foreach ($parts as $part)
{
	if ($part == "")
		continue;
	list($raw_headers, $body) = explode("\r\n\r\n", $part, 2);

	// 헤더 파싱
	$headers = [];
	foreach (explode("\r\n", $raw_headers) as $header)
	{
		list($name, $value) = explode(': ', $header);
		$headers[$name] = $value;
	}

	// Content-Disposition에서 filename을 추출
	$match = preg_match('/filename="(.*)"/', $headers['Content-Disposition'], $filename_matches);
	if ($match == 0)
		$result .= "filename missed";
	$filename = $filename_matches[1];

	// 파일 내용을 저장
	$file_handle = fopen($filename, 'w');
	fwrite($file_handle, substr($body, 0, -2));
	fclose($file_handle);

	$result .= "File '{$filename}' uploaded successfully.\nTrgt={$filename}";
}

echo $result;
?>