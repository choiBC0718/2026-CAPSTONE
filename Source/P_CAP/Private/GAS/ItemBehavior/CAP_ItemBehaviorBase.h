// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "Interface/CAP_BehaviorStateProvider.h"
#include "CAP_ItemBehaviorBase.generated.h"

class UCAP_AbilitySystemComponent;
class UAbilitySystemComponent;
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
	virtual void OnEquipped(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const {}
	//아이템 해제 시 호출
	virtual void OnUnequipped(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const {}
	
protected:
	// 아이템이 장착될 때 트리거 태그를 들을 수 있도록 ASC에 태그 주파수 설정
	void BindGameplayEvent(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC, FGameplayTag EventTag) const;
	// 아이템이 해제되면 들을 필요가 없어지므로 주파수 연결 해제
	void UnbindGameplayEvents(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const;

	// 자식 클래스에서 실제 효과 로직을 구현
	virtual void OnEventReceived(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC, const struct FGameplayEventData* Payload) const {}
	// 쿨타임 체크, 통과 시 최근 발동 시간 갱신
	bool IsOnCooldown(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const;
	void ConsumeCooldown(ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const;
	
	void InitGameplayEffectToDefault(const FGameplayEffectSpecHandle& SpecHandle, TSubclassOf<UGameplayEffect> BuffGE, float DefaultVal=0.f) const;

	UPROPERTY(EditDefaultsOnly, meta=(ClampMin="0.0"))
	float Cooldown = 0.f;
	// 아이템 내부 여러 효과를 식별하기 위한 행동의 고유 태그 (한개의 아이템에 2개 Behavior 존재시, 각자 다른 태그로 설정해야 함)
	UPROPERTY(EditDefaultsOnly, meta=(Categories="Item.Behavior"))
	FGameplayTag BehaviorTag;
	
	FGameplayTag BaseDamageTag;
	FGameplayTag DamageMultiplierTag;
	FGameplayTag StackTag;
	FGameplayTag DurationTag;
	
private:
	// ASC 델리게이트와 연결될 내부 콜백 함수
	void InternalEventCallback(const struct FGameplayEventData* Payload, ICAP_BehaviorStateProvider* StateProvider, UCAP_AbilitySystemComponent* ASC) const;
};
