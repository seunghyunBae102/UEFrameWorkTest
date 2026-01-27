// Copyright 2024, Korstian Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CharacterStateTypes.generated.h"

/**
 * @enum ECharacterState
 * @brief 캐릭터의 현재 최상위 상태를 정의합니다.
 * 이 상태는 캐릭터가 어떤 행동을 할 수 있고 없는지를 결정하는 '규칙'으로 작용합니다.
 * 예를 들어, 'Attacking' 상태에서는 점프나 회피를 할 수 없습니다.
 */
UENUM(BlueprintType)
enum class ECharacterState : uint8
{
	/** @brief 기본 상태. 대부분의 행동을 시작할 수 있습니다. */
	Idle UMETA(DisplayName = "Idle"),
	/** @brief 달리는 중인 상태. 공격 등의 행동이 제한될 수 있습니다. */
	Sprinting UMETA(DisplayName = "Sprinting"),
	/** @brief 회피(구르기/백스텝) 중인 상태. 이 상태 동안에는 보통 무적(I-frame) 판정이 적용되며, 다른 모든 행동이 제한됩니다. */
	Dodging UMETA(DisplayName = "Dodging"),
	/** @brief 앉아있는 상태. 이동 속도가 느려지고 피격 범위가 줄어듭니다. */
	Crouching UMETA(DisplayName = "Crouching"),
	/** @brief 공격 중인 상태. 공격 애니메이션이 끝날 때까지 다른 대부분의 행동이 제한됩니다. */
	Attacking UMETA(DisplayName = "Attacking"),
	/** @brief 적의 공격에 맞아 경직된 상태. 플레이어의 모든 입력이 무시됩니다. */
	Stunned UMETA(DisplayName = "Stunned"),
	/** @brief 죽은 상태. 어떠한 행동도 할 수 없습니다. */
	Dead UMETA(DisplayName = "Dead"),

	// --- Parkour ---
	/** @brief 낮은 장애물을 넘어가는 중인 상태. 루트 모션 애니메이션이 재생되며 행동이 제한됩니다. */
	Vaulting UMETA(DisplayName = "Vaulting"),
	/** @brief 벽을 오르거나 매달려 이동하는 상태. */
	Climbing UMETA(DisplayName = "Climbing"),
	/** @brief 렛지(모서리)에 매달려 있는 상태. */
	Hanging UMETA(DisplayName = "Hanging"),

	// --- Combat ---
	/** @brief 가드(막기) 중인 상태. 이동이 제한되고 정면에서의 피해를 감소시킵니다. */
	Guarding UMETA(DisplayName = "Guarding"),
	/** @brief 패링을 시도 중인 상태. 짧은 시간 동안 특정 공격을 쳐낼 수 있습니다. */
	Parrying UMETA(DisplayName = "Parrying"),
};

/**
 * @enum ECharacterStance
 * @brief 캐릭터의 자세를 정의합니다.
 * 주로 애니메이션 블루프린트에서 서 있을 때와 앉아있을 때의 다른 애니메이션 세트(Locomotion)를 선택하는 데 사용됩니다.
 * 게임플레이 규칙보다는 시각적 표현에 더 큰 영향을 줍니다.
 */
UENUM(BlueprintType)
enum class ECharacterStance : uint8
{
	/** @brief 서 있는 자세 */
	Standing UMETA(DisplayName = "Standing"),
	/** @brief 앉아있는 자세 */
	Crouching UMETA(DisplayName = "Crouching"),
};
