# PointAndClickCustomizing 플러그인
<p align="center">
  <img src="https://github.com/user-attachments/assets/f0e4a85f-1fc7-4305-b1ac-8e575aa66fac" width="300" style="margin-right: 20px;" />
  <img src="https://github.com/user-attachments/assets/01aea4cc-7f9b-4c02-903c-9325319647fb" width="300" />
</p>

 ## 소개 (Introduction)
 1. 커스터마이징 기능 제공 : 마우스로 3D 파츠를 드래그 드롭으로 부착하고 회전하는 기능을 지원합니다.
 2. 기본 UI 제공 : 커스터마이징 파츠를 선택하고, 레벨을 전환할 수 있는 UI를 기본 제공합니다.
 3. 네트워크 동기화 제공 : 리슨 서버 환경에서 데이터를 서버 권위로 동기화하고, 투사체의 경우 클라이언트 측 예측을 하는 기능을 제공합니다.
 4. 로비 기능 제공 : 리슨 서버 참여자 중 로컬 사용자가 Ready 신호를 주고 이를 서버측에서 처리하여 레벨 전환하는 시스템을 제공합니다.
 5. 기본 전투 데모 제공 : 리슨 서버 환경에서 부착한 파츠가 Shootable 할 경우 투사체를 소환 발사하고 죽는 기본 사이클의 데모 레벨이 제공됩니다.


- **버전 1.2 변경점 :**
- 관련 UI가 슬레이트 기반으로 변경되었습니다.
- attach/remove 로직이 서버에서 먼저 처리 된 후 클라이언트에 적용됩니다.
- rotation 로직이 서버에서 거부시 client에서 초기 rotation으로 롤백됩니다.
- 레벨 전환 시 Ready 여부가 PopUp UI 시스템에 의해서 제공됩니다.
- 배틀 레벨에서, Top View로 투사체를 발사하는 로직을 테스트 할 수 있습니다. (Spherer 액터 부착 시 테스팅 가능)
- 배틀 레벨에서, 클라이언트 측 예측 (프록시 투사체를 클라이언트 단에 생성하여 즉각 발사를 보여주는 로직)이 추가되었습니다.

## 커스터마이징 세부 기능 ##

- **프리뷰 스폰 & 스냅**  
  - `RequestSpawnByID(ID)` → PreviewActor가 마우스 위치에 스폰  
  - 근접한 본/소켓에 **스냅**  
- **아이템 회전 (Arcball)**  
  - `RotateFocusedActor(prev, curr, viewport, speed)`  
  - 포커스된 액터를 마우스 드래그로 정밀 회전  
- **포커스 & 삭제**  
  - Idle 상태에서 `TryFocusAttachedActor()`  
  - 포커스된 상태에서 `DeleteFocusedActor(PlayerID)`  
- **확정 & 취소**  
  - Snap 상태에서 `FinalizeAttachment()` → 본격 부착 + 서버 저장  
  - `CancelPreview()`, `CancelFocus()`  
- **Load / Save**  
  - `LoadExistingAttachments(PlayerID)`  
  - 모든 부착 기록을 서버-클라이언트 동기화  
- **Ready Button**  
  - `PressReadyButton()` → `Server_SendReady` RPC → `ReadyGameMode` 전환

---




## 설치 (Installation)
1. 프로젝트 루트에 **Plugins** 폴더를 만들고, `PointAndClickCustomizing` 폴더를 통째로 넣습니다.  
2. 언리얼 에디터에서 **Edit > Plugins** 를 열고 **PointAndClickCustomizing** 플러그인을 활성화한 뒤 재시작합니다.  
3. `Build.cs` 또는 `.uplugin` 에 **EnhancedInput**, **OnlineSubsystem** 모듈이 Dependencies에 포함되어 있어야 합니다.

## 사용법 (How to Use)
1. **캐릭터 구성:**  
   - `ACustomCharacter` 또는 `ICustomizableCharacter`를 구현한 캐릭터에 `UCustomizingActorComponent` 추가.  
2. **컨트롤러 지정:**  
   - 커스터마이징 모드의 GameMode에서 PlayerController를 `ACustomizingPlayerController`로 설정.  
   - `CustomizingMappingContext`, `IA_Look`, `IA_Zoom` 등 InputAction 자산을 할당.  
3. **아이템 DataTable 준비:**  
   - `ActorDataTable`에 `FActorDataRow` 구조체 기반 DataTable 할당.  
4. **프리뷰 스폰:**  
   - `TrySpawningAttachableActor(FName ID)` 호출로 미리보기 액터 스폰.  
5. **회전/취소/확정:**  
   - `RotateFocusedActor()`, `CancelPreview()`, `FinalizeAttachment()` 호출로 부착 동작 제어.  
6. **편집/삭제:**  
   - 이미 부착된 아이템은 클릭 시 `TryFocusAttachedActor()`, 삭제 시 `DeleteFocusedActor()`.  
7. **준비 완료:**  
   - `PressReadyButton()` 호출로 서버에 Ready RPC 전송.  
   - 서버에서 `ReadyGameMode`가 모든 클라이언트 준비를 감지 후 `ServerTravel()` 호출.


---
## Introduction

The **PointAndClickCustomizing** plugin is an Unreal Engine extension that provides point-and-click character customization using attachable actors and a state machine. Players can attach items to their character with the mouse, adjust position and rotation, then press the “Ready” button to finalize customization. Once every client is ready, the game automatically transitions to the next level. This plugin manages customization steps via a dedicated state machine component and supports server-client synchronization in multiplayer (listen server) setups.

---

## Features

- **Ready Button**  
  When pressed after finishing customization, signals the server that the player is ready.

- **Game Mode Transition**  
  Using `ReadyGameMode`, once all players are ready, smoothly switches from the customization phase to the main game mode.

- **Mesh Pivot Rotation**  
  Precisely rotate attached items around their pivot via mouse drag controls.

- **Preview & Finalize Attachment**  
  Spawn an item in preview mode to adjust its position, then call `FinalizeAttachment()` to attach it permanently.

- **Server-Client Synchronization**  
  All attachment logic runs on the server, and final results are saved server-side and replicated to clients.

- **Modular Design**  
  Separation of concerns via a gateway component and multiple sub-components for clear responsibility division.

---
```txt
UAttachmentDataStore
           │
UCustomizingActorComponent  ←── Gateway API ──→  PlayerController
           │
           ├── UAttachmentPreviewComponent    (Preview·Snap·Move·Finalize)
           ├── UAttachmentPersistenceComponent (Load/Save RPC)
           └── UAttachmentFocusComponent      (Click and Focus Actor · Record Properties · Delete Record)
           └── UAttachmentRotationComponent   (Rotation · Save)
```
## Details

- **Preview Spawn & Snap**  
  - `RequestSpawnByID(ID)` → Spawns a preview actor at the mouse location  
  - Snaps to the nearest bone/socket  

- **Item Rotation (Arcball)**  
  - `RotateFocusedActor(prev, curr, viewport, speed)`  
  - Precisely rotate the focused actor with mouse drag  

- **Focus & Delete**  
  - In Idle state: `TryFocusAttachedActor()`  
  - In Focused state: `DeleteFocusedActor(PlayerID)`  

- **Finalize & Cancel**  
  - In Snapped state: `FinalizeAttachment()` → permanent attach + server save  
  - Cancel operations: `CancelPreview()`, `CancelFocus()`  

- **Load / Save**  
  - `LoadExistingAttachments(PlayerID)`  
  - Synchronizes all attachment records between server and clients  

- **Ready Button**  
  - `PressReadyButton()` → `Server_SendReady` RPC → triggers `ReadyGameMode` transition  

---

## Installation

1. Create a **Plugins** folder at your project root and copy the `PointAndClickCustomizing` folder into it.  
2. In Unreal Editor, go to **Edit > Plugins**, enable **PointAndClickCustomizing**, then restart the editor.  
3. Ensure **EnhancedInput** and **OnlineSubsystem** modules are listed in your `Build.cs` or `.uplugin` dependencies.  

---

## How to Use

1. **Character Setup:**  
   - Add `UCustomizingActorComponent` to your `ACustomCharacter` or any `ICustomizableCharacter` implementation.  

2. **Controller Setup:**  
   - In your customization GameMode, set the PlayerController class to `ACustomizingPlayerController`.  
   - Assign `CustomizingMappingContext`, `IA_Look`, `IA_Zoom`, and other InputAction assets.  

3. **Prepare Item DataTable:**  
   - Create a DataTable based on `FActorDataRow` and assign it to `ActorDataTable`.  

4. **Spawn Preview:**  
   - Call `TrySpawningAttachableActor(FName ID)` to spawn the preview actor.  

5. **Rotate / Cancel / Finalize:**  
   - Use `RotateFocusedActor()`, `CancelPreview()`, and `FinalizeAttachment()` to control attachments.  

6. **Edit / Delete:**  
   - Focus an attached item with `TryFocusAttachedActor()`, then delete with `DeleteFocusedActor()`.  

7. **Ready Up:**  
   - Call `PressReadyButton()` to send the Ready RPC.  
   - When all clients are ready, `ReadyGameMode` will detect and call `ServerTravel()`.  

---

## Contribution & Future Plans

- **Contributions Welcome:** Open PRs and issues! Please follow code style (logging, comments, memory management).  
- **Future Plans:**  
  - Additional UI widgets  
  - Persistent SaveGame support  
  - Extended customization options (color, scale)  
  - Enhanced Ready system  
  - Performance and engine updates compatibility  
