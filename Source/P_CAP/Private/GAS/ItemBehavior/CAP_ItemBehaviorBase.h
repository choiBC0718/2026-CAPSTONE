// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "CAP_ItemBehaviorBase.generated.h"

class UAbilitySystemComponent;
class UCAP_ItemInstance;
/**
 * 아이템의 개별 특수 효과 모듈
 * EditInlineNew + DefaultToInstanced로 DA에서 조립하도록
 * Data Asset에 들어가 메모리를 공유하게 됨 -> 런타임 상태값은 고유 인스턴스 Item_Instance에 저장 필요
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced, BlueprintType, CollapseCategories)
class UCAP_ItemBehaviorBase : public UObject
{
	GENERATED_BODY()

public:
	UCAP_ItemBehaviorBase();
	//아이템 장착 시 호출  
	virtual void OnEquipped(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const;
	//아이템 해제 시 호출
	virtual void OnUnequipped(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const;

	UPROPERTY(EditDefaultsOnly, meta=(ClampMin="0.0"))
	float Cooldown = 0.f;
	
protected:
	// 아이템이 장착될 때 트리거 태그를 들을 수 있도록 ASC에 태그 주파수 설정
	void BindGameplayEvent(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC, FGameplayTag EventTag) const;
	// 아이템이 해제되면 들을 필요가 없어지므로 주파수 연결 해제
	void UnbindGameplayEvents(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const;

	// 자식 클래스에서 실제 효과 로직을 구현
	virtual void OnEventReceived(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC, const struct FGameplayEventData* Payload) const {}
	// 쿨타임 체크, 통과 시 최근 발동 시간 갱신
	bool IsOnCooldown(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const;
	void ConsumeCooldown(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const;
	
	void InitGameplayEffectToZero(const FGameplayEffectSpecHandle& SpecHandle, TSubclassOf<UGameplayEffect> BuffGE) const;

	FGameplayTag BaseDamageTag;
	FGameplayTag DamageMultiplierTag;
	FGameplayTag StackTag;
	FGameplayTag DurationTag;
	
private:
	// ASC 델리게이트와 연결될 내부 콜백 함수
	void InternalEventCallback(const struct FGameplayEventData* Payload, UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const;
};
