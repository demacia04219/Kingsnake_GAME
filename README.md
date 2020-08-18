# Kingsnake_GAME

실시간 모바일 1:1 대전액션 게임 "Kingsnake"의 게임서버입니다.

### 프로그램 설명
한국IT직업전문학교에서의 2018 G-STAR 출품 게임 "Kingsnake"의 서비스를 위해 개발 된 프로그램입니다.

    - 로그인
    - 로비(캐릭터, 아이템 변경) 기능
    - 게임 대기열 등록
    - 게임 진행
    - 결과 전송

### 기술
- C++(MSVC) + STL(Container)
- Multi-thread
- Network with TCP/IP & IOCP
- Windows API(winsock)

### 역할
- 로그인 데이터 DB서버로 전송하여 검증
- Lobby 화면으로 유저 데이터 전송
- 게임 대기열 관리
- 게임 세션 관리
- 게임 세션 Loop 실행
- 게임 결과 클라이언트와 DB서버로 전송
