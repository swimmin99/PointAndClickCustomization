# PointAndClickCustomizing 플러그인
<p align="center">
  <img src="https://github.com/user-attachments/assets/f0e4a85f-1fc7-4305-b1ac-8e575aa66fac" width="300" style="margin-right: 20px;" />
  <img src="https://github.com/user-attachments/assets/01aea4cc-7f9b-4c02-903c-9325319647fb" width="300" />
</p>

 ## 소개 (Introduction)
PointAndClickCustomizing 플러그인은 액터 부착 및 상태 머신을 활용하여 포인트 앤 클릭 방식의 캐릭터 커스터마이징 기능을 제공하는 언리얼 엔진 플러그인입니다.  
플레이어는 마우스로 아이템을 캐릭터에 부착하고 위치/회전을 조정한 뒤 준비(Ready) 버튼을 눌러 커스터마이징을 확정할 수 있습니다. 각 클라이언트의 준비가 완료되면 자동으로 다음 레벨로 이동됩니다.
이 플러그인은 커스터마이징 기능을 전용 상태 머신 컴포넌트로 단계를 관리하며, 멀티플레이어 환경에서의 서버-클라이언트 동기화를 지원합니다. 리슨 서버를 지원하도록 제작되었습니다.

- **버전 1.2 변경점 :**
- 관련 UI가 슬레이트 기반으로 변경되었습니다.
- attach/remove 로직이 서버에서 먼저 처리 된 후 클라이언트에 적용됩니다.

## 주요 기능 (Features)
- **준비(Ready) 버튼:** 커스터마이징 완료 시 누르는 버튼으로, 플레이어의 준비 완료를 서버에 알립니다.  
- **게임 모드 전환 (ReadyGameMode 지원):** 모든 플레이어가 준비되면 커스터마이징 전용 모드에서 본 게임 모드로 부드럽게 전환합니다.  
- **메쉬 피벗 회전 기능:** 부착 아이템의 피벗을 기준으로 마우스 드래그를 통해 세밀하게 회전시킬 수 있습니다.  
- **부착 미리보기 및 확정:** 아이템을 프리뷰 모드로 스폰하여 위치를 조정한 뒤 서버에 부착 요청을 보냅니다.  
- **서버-클라이언트 동기화:** 모든 부착 로직은 서버 권한으로 실행되고, 최종 부착 결과는 서버에 저장되어 모든 클라이언트에 복제됩니다.  
- **서브 컴포넌트와 게이트웨이 컴포넌:** 분리 설계로 책임 분산을 특징으로 합니다.  

## 아키텍쳐

```txt
UCustomizingActorComponent  ←── Gateway API ──→  PlayerController
           │
           ├── UAttachmentPreviewComponent    (프리뷰·스냅·이동·확정)
           ├── UAttachmentPersistenceComponent (Load/Save RPC)
           └── UAttachmentFocusComponent      (클릭 포커스·기록 설정·삭제)
                      └── UAttachmentRotationComponent   (Arcball 회전)

(Base) UCustomizingSubBaseComponent
  └─ lazy-cache: StateMachine / Character / Mesh / FocusComp / DataTable / CurrentRecord
```

## 세부 기능 ##

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

## 기여 및 향후 계획 (Contribution and Future Plans)
- **기여:** PR, 이슈 제보 환영! 코드 스타일(로그, 영어 주석, 메모리 관리 등)을 준수 부탁드립니다.  
- **향후 계획:**  
  - 추가 UI 위젯 제공  
  - SaveGame 기반 영구 저장/불러오기  
  - 색상/스케일 조정 등 확장 커스터마이징
  - Ready 기능 고도화
  - 최적화 및 엔진 업데이트 대응

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
UCustomizingActorComponent  ←── Gateway API ──→  PlayerController
           │
           ├── UAttachmentPreviewComponent    (Preview·Snap·Move·Finalize)
           ├── UAttachmentPersistenceComponent (Load/Save RPC)
           └── UAttachmentFocusComponent      (Click and Focus Actor · Record Properties · Delete Record)
                      └── UAttachmentRotationComponent   (Arcball Based Actor Rotation)
           
(Base) UCustomizingSubBaseComponent
  └─ lazy-cache: StateMachine / Character / Mesh / FocusComp / DataTable / CurrentRecord
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
