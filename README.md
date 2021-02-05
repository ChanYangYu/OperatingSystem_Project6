# OperatingSystem_Project6
## 과제 목표
- 가상 디스크(애뮬레이터)를 위한 간단한 파일 시스템 및 기본 파일 operation 구현
- 디스크 접근 기능이 구현된 ssufs-disk.h와 ssufs-disk.c에서 정의(구현된)된 여러 함수를 이용하여,
ssufs-ops.c를 수정(ssufs_create(), ssufs_open(), ssufs_delete(), ssufs_read(), ssufs_write() 구현)

## 과제 구현 내용
#### 1. int ssufs_create(char *filename)
- 파일 시스템에 요청된 filename으로 파일 생성
- 파일 생성은 동일한 이름의 파일이 파일 시스템에 존재하지 않고 시스템에 추가로 파일을 생성할 공간이 있는 경우에만 성공해야 함
- 해당 파일을 위해 사용 가능한 inode를 할당하고 디스크에서 적절하게 초기화 필요
- 성공 시 : 새로 생성 된 파일의 inode 번호를 리턴
- 에러 시 : -1 리턴
#### 2. void ssufs_delete(char *filename)
- 파일 시스템에서 요청된 filename으로 파일 삭제 (존재하는 경우에만)
- inode와 같은 해당 파일과 관련된 리소스를 해제해야 함
- close()를 하지 않아도 삭제해도 됨

#### 3. int ssufs_open(char *filename)
- 파일 열기
- create()로 생성된 파일의 경우에만 성공
- file handle 배열에서 사용하지 않는 파일 핸들의 한 항목을 할당하고 초기화한 다음 새로 할당된 file handle의 인덱스 리턴
- 에러 시 : –1 리턴
- 참고 : 파일의 읽기 및 쓰기는 본 함수로 리턴된 file handle 인덱스를 사용하여 파일의 inode 번호를 참조해야 함

#### 4. int ssufs_read(int file handle, char *buf, int nbytes)
- open()된 파일의 현재 오프셋에서 요청된 buf로 요청된 nbytes 수를 읽음
- 요청된 nbytes 수는 디스크 상에서 연속되지 않을 수도 있음 -> 여러 블록에 걸쳐 있을 수 있음
- 성공 시 : 0 리턴
- 에러 시 : -1 리턴

#### 5. int ssufs_write(int file handle, char *buf, int nbytes)
- open()된 파일의 현재 오프셋에서 요청된 buf에서 요청된 nbytes 수를 디스크에 씀
- 요청된 nbytes 수는 디스크 상에서 연속되지 않을 수도 있음 -> 여러 블록에 걸쳐있을 수 있음
- 성공 시 : 0 리턴
- 에러 시 : -1 리턴
