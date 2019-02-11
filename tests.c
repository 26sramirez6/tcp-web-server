/*
 * tests.c
 *
 *  Created on: Feb 10, 2019
 *      Author: sramirez266
 */
#include <assert.h>
#include "response.h"

static void
test1() {
	char * req = "OPTIONS /images/uchicago/logo.png HTTP/1.1\r\nHost: www.someschool.edu\r\n";
	HTTP_response_t response;
	HTTP_request_t request;
	request.buf = (byte *) req;
	InitializeResponse(&response);
	FillResponse(&request, &response);
	assert(response.respCode==RESPONSE_NOT_ALLOWED);
	printf("%s\n", response.buf->buf);
	FreeResponse(&response);
}

static void
test2() {
	char * req = "GET /images/uchicago/logo.png HTTP/1.1\r\nHost: www.someschool.edu\r\n";
	HTTP_response_t response;
	HTTP_request_t request;
	request.buf = (byte *) req;
	InitializeResponse(&response);
	FillResponse(&request, &response);
	assert(response.respCode==RESPONSE_OK);
	printf("%s\n", response.buf->buf);
	FreeResponse(&response);
}

static void
test3() {
	char * req = "GET /file/notfound.png HTTP/1.1\r\nHost: www.someschool.edu\r\n";
	HTTP_response_t response;
	HTTP_request_t request;
	request.buf = (byte *) req;
	InitializeResponse(&response);
	FillResponse(&request, &response);
	assert(response.respCode==RESPONSE_NOT_FOUND);
	printf("%s\n", response.buf->buf);
	FreeResponse(&response);
}

static void
test4() {
	char * req = "HEAD /file/notfound.png HTTP/1.1\r\nHost: www.someschool.edu\r\n";
	HTTP_response_t response;
	HTTP_request_t request;
	request.buf = (byte *) req;
	InitializeResponse(&response);
	FillResponse(&request, &response);
	assert(response.respCode==RESPONSE_NOT_FOUND);
	printf("%s\n", response.buf->buf);
	FreeResponse(&response);
}

static void
test5() {
	char * req = "HEAD /images/uchicago/logo.png HTTP/1.1\r\nHost: www.someschool.edu\r\n";
	HTTP_response_t response;
	HTTP_request_t request;
	request.buf = (byte *) req;
	InitializeResponse(&response);
	FillResponse(&request, &response);
	assert(response.respCode==RESPONSE_OK);
	printf("%s\n", response.buf->buf);
	FreeResponse(&response);
}

static void
test6() {
	char * req = "HEAD _ /file/notfound.png HTTP/1.1\r\nHost: www.someschool.edu\r\n";
	HTTP_response_t response;
	HTTP_request_t request;
	request.buf = (byte *) req;
	InitializeResponse(&response);
	FillResponse(&request, &response);
	assert(response.respCode==RESPONSE_BAD);
	printf("%s\n", response.buf->buf);
	FreeResponse(&response);
}

static void
test7() {
	char * req = "HEAD /redirect.defs HTTP/1.1\r\nHost: www.someschool.edu\r\n";
	HTTP_response_t response;
	HTTP_request_t request;
	request.buf = (byte *) req;
	InitializeResponse(&response);
	FillResponse(&request, &response);
	assert(response.respCode==RESPONSE_NOT_FOUND);
	printf("%s\n", response.buf->buf);
	FreeResponse(&response);
}

static void
test8() {
	char * req = "GET /cats HTTP/1.1\r\nHost: www.someschool.edu\r\n";
	HTTP_response_t response;
	HTTP_request_t request;
	request.buf = (byte *) req;
	InitializeResponse(&response);
	FillResponse(&request, &response);
	assert(response.respCode==RESPONSE_MOVED);
	printf("%s\n", response.buf->buf);
	FreeResponse(&response);
}

int main() {
	printf("==========TEST 1:==========\n");
	test1();
	printf("==========TEST 2:==========\n");
	test2();
	printf("==========TEST 3:==========\n");
	test3();
	printf("==========TEST 4:==========\n");
	test4();
	printf("==========TEST 5:==========\n");
	test5();
	printf("==========TEST 6:==========\n");
	test6();
	printf("==========TEST 7:==========\n");
	test7();
	printf("==========TEST 8:==========\n");
	test8();
	FreeRedirects();
	printf("exiting\n");
}
