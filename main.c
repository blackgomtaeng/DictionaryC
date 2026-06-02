#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dict.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

int main() {
#if defined(_WIN32) || defined(_WIN64)
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
#endif

    DictData my_data;
    memset(&my_data, 0, sizeof(DictData));

    printf("===========================================\n");
    printf("   동적 라이브러리 검증 및 보안 보고 콘솔\n");
    printf("===========================================\n");
    printf("인증 코드 입력: ");
    scanf("%31s", my_data.auth_code);

    printf("사전 종류 선택 (1:국어, 2:법률): ");
    if (scanf("%d", &my_data.dict_type) != 1) return 1;

    printf("검색 용어 입력: ");
    scanf("%127s", my_data.keyword);

    strcpy(my_data.client_ip, "127.0.0.1");
    my_data.client_port = 50001;

    const char *server_target_ip = "127.0.0.1";
    int server_target_port = 9999;

    printf("\n[이벤트] 인증 검증 및 동적 통신 함수를 트리거합니다...\n");
    int status = execute_dict_dll(&my_data, server_target_ip, server_target_port);

    if (status == 0) {
        printf("\n✅ [처리 및 상태 보고 완료]");
        printf("\n- 검색 단어: %s", my_data.keyword);
        printf("\n- 파싱 결과: %s\n", my_data.definition);
    } else {
        printf("\n❌ 처리 실패 또는 비인가 차단! 에러코드: %d\n", status);
        if(status == -9) {
            printf("정의된 메인 결과: %s\n", my_data.definition);
        }
    }

    return 0;
}
