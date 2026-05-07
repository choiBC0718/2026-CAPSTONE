#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseMonster.generated.h"

class USphereComponent;

UCLASS()
class P_CAP_API ABaseMonster : public ACharacter
{
	GENERATED_BODY()

public:
	ABaseMonster();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Zone")
	USphereComponent* OuterAttackZone;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat|Zone")
	USphereComponent* InnerAttackZone;

	// [변경] 하드코딩 제거 → 에디터에서 몬스터별로 설정 가능
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Zone", meta=(ClampMin="50.0"))
	float OuterAttackRadius = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Zone", meta=(ClampMin="10.0"))
	float InnerAttackRadius = 200.f;

	UFUNCTION(BlueprintCallable, Category = "Monster")
	void Die();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ReceiveAttack(AActor* Attacker);

protected:
	virtual void BeginPlay() override;
};