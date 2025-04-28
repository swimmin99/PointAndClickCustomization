# PointAndClickCustomizing 플러그인
<p align="center">
  <img src="https://github.com/user-attachments/assets/f0e4a85f-1fc7-4305-b1ac-8e575aa66fac" width="300" style="margin-right: 20px;" />
  <img src="https://github.com/user-attachments/assets/01aea4cc-7f9b-4c02-903c-9325319647fb" width="300" />
</p>
## 소개 (Introduction)
본 프로젝트는 스마일게이트 언리얼 교육 프로그램 UNSEEN 2기 시연회 출품 작품의
멀티플레이어 완성 구현 버전을 플러그인화한 것입니다. 핵심 로직과 기능 시연에 중점을 두었기 때문에, 그 외 부분은 예시 수준으로 구현되어 있습니다.

PointAndClickCustomizing 플러그인은 액터 부착 및 상태 머신을 활용하여 포인트 앤 클릭 방식의 캐릭터 커스터마이징 기능을 제공하는 언리얼 엔진 플러그인입니다.  

플레이어는 마우스로 아이템을 캐릭터에 부착하고 위치/회전을 조정한 뒤 준비(Ready) 버튼을 눌러 커스터마이징을 확정할 수 있습니다. 각 클라이언트의 준비가 완료되면 자동으로 다음 레벨로 이동됩니다.

이 플러그인은 커스터마이징 기능을 전용 상태 머신 컴포넌트로 단계를 관리하며, 멀티플레이어 환경에서의 서버-클라이언트 동기화를 지원합니다. 리슨 서버를 지원하도록 제작되었습니다.

## 주요 기능 (Features)
- **준비(Ready) 버튼:** 커스터마이징 완료 시 누르는 버튼으로, 플레이어의 준비 완료를 서버에 알립니다.  
- **게임 모드 전환 (ReadyGameMode 지원):** 모든 플레이어가 준비되면 커스터마이징 전용 모드에서 본 게임 모드로 부드럽게 전환합니다.  
- **메쉬 피벗 회전 기능:** 부착 아이템의 피벗을 기준으로 마우스 드래그를 통해 세밀하게 회전시킬 수 있습니다.  
- **부착 미리보기 및 확정:** 아이템을 프리뷰 모드로 스폰하여 위치를 조정한 뒤 `FinalizeAttachment()` 호출로 본격 부착합니다.  
- **서버-클라이언트 동기화:** 모든 부착 로직은 서버 권한으로 실행되고, 최종 부착 결과는 서버에 저장되어 모든 클라이언트에 복제됩니다.  
- **상태 머신 컴포넌트 기반 흐름:** `UStateMachineComponent`로 **Idle**, **ActorMoving**, **ActorSnapped**, **ActorFocused**, **ActorCanFocus** 등 상태 전환을 관리합니다.



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

# PointAndClickCustomizing Plugin

## Introduction
This project is a plugin adaptation of the fully-featured multiplayer build from the Smilegate Unreal Education Program UNSEEN Season 2 demo submission.
As its primary aim is to showcase core logic and functionality, all other areas are implemented at an illustrative, example level.  Architecture is specifically designed to support a listen server setup.

PointAndClickCustomizing is an Unreal Engine plugin that enables point-and-click character customization through attachable actors and a state machine system. It allows players to attach items to their character with mouse input, adjust their position/rotation, and then confirm the customization by clicking a “Ready” button. The plugin manages the customization flow with a dedicated state machine component and supports seamless server-client synchronization in multiplayer environments.

## Features
- **Ready Button:** Finalizes the customization session and signals the server that the player is ready to proceed.  
- **Game Mode Transition:** Upon all players being ready, smoothly transitions from the customization phase to the main gameplay phase using ReadyGameMode.  
- **Customizable Mesh Pivot Rotation:** Enables precise rotation of attached items around their pivot via intuitive mouse drag controls.  
- **Attachable Preview and Attachment:** Spawns a preview actor for positioning before permanently attaching it to the character’s bone/socket with `FinalizeAttachment()`.  
- **Server-Client Synchronization:** All spawn/attach operations are authoritative on the server, and the final attachment data is saved server-side and replicated to all clients.  
- **State Machine Component Driven Flow:** Manages states—Idle, ActorMoving, ActorSnapped, ActorFocused, ActorCanFocus—via a dedicated state machine component for clear, maintainable logic.

## Installation
1. **Add the Plugin:** Place the `PointAndClickCustomizing` folder into your project’s `Plugins` directory.  
2. **Enable in Editor:** In Unreal Editor, go to **Edit > Plugins** and enable **PointAndClickCustomizing**, then restart.  
3. **Dependencies:** Ensure **Enhanced Input** and **OnlineSubsystem** modules are enabled and listed in your `Build.cs` or `.uproject`.

## How to Use
1. **Attach Customizing Component:** Add `UCustomizingActorComponent` to your player character (or use `ACustomCharacter`).  
2. **Set PlayerController:** In your customization GameMode, set the controller to `ACustomizingPlayerController`. Configure its `CustomizingMappingContext` and `UInputAction` properties (IA_Look, IA_Zoom, etc.).  
3. **Prepare DataTable:** Assign a `UDataTable` asset to the component’s `ActorDataTable` property, matching the `FActorDataRow` struct.  
4. **Spawn Preview:** Call `TrySpawningAttachableActor(FName ID)` to spawn a preview actor that snaps to valid bones.  
5. **Adjust & Finalize:** Rotate (`RotateFocusedActor`), cancel (`CancelPreview()`), or finalize (`FinalizeAttachment()`) attachments.  
6. **Edit/Remove:** Focus existing attachments with `TryFocusAttachedActor()`, delete with `DeleteFocusedActor()`, and cancel focus with `CancelFocus()`.  
7. **Ready Up:** Press the Ready button (e.g., `PressReadyButton()`) to send `Server_SendReady` RPC. Once all players are ready, your GameMode can transition levels or modes.

## Contribution and Future Plans
- **Contributions:** PRs and issues are welcome—please follow the plugin’s code style and logging conventions.  
- **Future Plans:** Add more UI widgets, persistent save/load, extended customization options (color, scale), and performance/network optimizations and better ready system.  
