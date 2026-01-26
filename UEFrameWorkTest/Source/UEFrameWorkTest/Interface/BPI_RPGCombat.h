// GHOST IN THE MONITOR - RPG Combat Interface
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BPI_RPGCombat.generated.h"

/**
 * RPG 데미지 정보 구조체
 */
USTRUCT(BlueprintType)
struct FRPGDamageInfo
{
	GENERATED_BODY()

	// 물리 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PhysicalDamage = 0.0f;

	// 마법 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MagicDamage = 0.0f;

	// 강인도 피해 (추가 피해)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float PoiseDamage = 0.0f;

	// 공격자 (누가 때렸는가)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* Attacker = nullptr;

	// 공격 타입 (정상 공격, 치명타, 약점 공격 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName DamageType = TEXT("Normal");
};

/**
 * BPI_RPGCombat - Inner World 전투 인터페이스
 * 
 * 역할:
 * - RPG 캐릭터/몬스터가 구현해야 할 전투 인터페이스
 * - 피격 판정, 강인도 처리, 사망 등을 정의
 * - Cast 없이 Interface Call로만 통신
 * 
 * 규칙:
 * - 반환값 없음: 모든 결과는 Delegate로 전파
 * - 구현은 BPC_RPGStats와 같은 Component에서 담당
 * - 클라이언트는 Interface만 알고 있으면 됨
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UBP_RPGCombat : public UInterface
{
	GENERATED_BODY()
};

class IBP_RPGCombat
{
	GENERATED_BODY()

public:
	/**
	 * RPG 데미지 수신
	 * @param DamageInfo: 데미지 정보 (물리/마법/강인도)
	 * 
	 * 로직:
	 * 1. 강인도 피해 적용
	 * 2. 강인도가 0이 되면 Stagger 상태 전환
	 * 3. 물리/마법 데미지 적용 (체력 감소)
	 * 4. 체력이 0 이하면 OnDeath 이벤트 발생
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RPGCombat")
	void TakeRPGDamage(const FRPGDamageInfo& DamageInfo);

	/**
	 * 현재 체력 조회
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RPGCombat")
	float GetCurrentHealth() const;

	/**
	 * 최대 체력 조회
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RPGCombat")
	float GetMaxHealth() const;

	/**
	 * 캐릭터가 살아있는지 확인
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RPGCombat")
	bool IsAlive() const;

	/**
	 * 강인도 정보 조회
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RPGCombat")
	float GetCurrentPoise() const;

	/**
	 * Stagger 상태인지 확인
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RPGCombat")
	bool IsStaggered() const;

	/**
	 * Monitor Hit에서 오는 물리적 충돌 수신 (Meta Impact)
	 * Outer World의 총기 발사가 Inner World로 영향을 미칠 때 사용
	 * 
	 * @param ImpactPoint: Inner World의 충돌 위치
	 * @param Force: 물리적 힘의 크기
	 * @param RadialRadius: 영향 반경
	 * 
	 * 로직:
	 * 1. 충돌점 주변의 모든 물리 객체에 충격 가함
	 * 2. 강인도 데미지 추가 적용 (선택사항)
	 * 3. 연쇄 반응 (추가 물체 파괴 등)
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RPGCombat")
	void ReceiveMetaImpact(FVector ImpactPoint, float Force, float RadialRadius);

	/**
 * @brief Applies poise damage to the character.
 * @param DamageAmount The amount of poise damage to apply.
 * @param HitResult Detailed information about the impact.
 * @param DamageCauser The actor that caused this damage.
 */
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RPGCombat|Poise")
void ApplyPoiseDamage(float DamageAmount, const FHitResult& HitResult, AActor* DamageCauser);
	

/** @brief Returns the maximum poise value of the character. */
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "RPGCombat|Poise")
float GetMaxPoise() const;

/** @brief Called when the character's poise is broken and they enter a groggy state. */
UFUNCTION(BlueprintImplementableEvent, Category = "RPGCombat|Poise")
void OnPoiseBroken();

/** @brief Called when the character is staggered by an attack. */
UFUNCTION(BlueprintImplementableEvent, Category = "RPGCombat|Poise")
void OnStagger();
};
