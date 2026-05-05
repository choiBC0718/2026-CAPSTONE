// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/ItemBehavior/ItemBehavior_ApplyBuffToSelf.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Character/Player/CAP_PlayerCharacter.h"
#include "Component/CAP_InventoryComponent.h"
#include "Data/CAP_AbilitySystemGenerics.h"
#include "GAS/CAP_AbilitySystemComponent.h"
#include "Interactables/Item/CAP_ItemInstance.h"

void UItemBehavior_ApplyBuffToSelf::OnEquipped(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const
{
	Super::OnEquipped(ItemInst,ASC);
	BindGameplayEvent(ItemInst,ASC,TriggerEventTag);
}

void UItemBehavior_ApplyBuffToSelf::OnUnequipped(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC) const
{
	UnbindGameplayEvents(ItemInst,ASC);
	Super::OnUnequipped(ItemInst,ASC);
}

void UItemBehavior_ApplyBuffToSelf::OnEventReceived(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC, const struct FGameplayEventData* Payload) const
{
	if (!TargetStatTag.IsValid())
	{
		UE_LOG(LogTemp,Warning,TEXT("상승시킬 스탯의 태그 설정 필요"));
		return;
	}
	
	if (!CheckTriggerCondition(ItemInst,ASC))
		return;

	// 기존 스택 확인 & 파괴용 핸들
	FActiveGameplayEffectHandle ExistingHandle;
	int32 CurrentItemStack = GetExistingStackCount(ItemInst,ASC,ExistingHandle);
	// 최대치 넘지 않게 설정
	int32 TargetStackCount = FMath::Min(CurrentItemStack+1, MaxStackCount);

	// 지속 시간, 스택 갱신하기 위해 기존 GE 파괴
	if (ExistingHandle.IsValid())
	{
		ASC->RemoveActiveGameplayEffect(ExistingHandle);
	}
	
	ApplyBuffWithStack(ItemInst,ASC,TargetStackCount);

	if (ACAP_PlayerCharacter* Player = Cast<ACAP_PlayerCharacter>(ASC->GetAvatarActor()))
	{
		if (UCAP_InventoryComponent* InvComp = Player->GetInventoryComponent())
			InvComp->OnItemEffectTriggered.Broadcast(ItemInst,TriggerEventTag,Cooldown,Duration,TargetStackCount);
	}
}

bool UItemBehavior_ApplyBuffToSelf::CheckTriggerCondition(UCAP_ItemInstance* ItemInst,UAbilitySystemComponent* ASC) const
{
	if (IsOnCooldown(ItemInst,ASC))
		return false;
	if (FMath::RandRange(0.f,100.f)>TriggerChance)
		return false;
	
	int32& CurrentCount = ItemInst->BehaviorCounters.FindOrAdd(this);
	CurrentCount++;
	if (CurrentCount < RequiredTriggerCount)
		return false;

	ConsumeCooldown(ItemInst, ASC);
	CurrentCount=0;
	return true;
}

void UItemBehavior_ApplyBuffToSelf::ApplyBuffWithStack(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC, int32 TargetStackCount) const
{
	UCAP_AbilitySystemComponent* CAP_ASC = ItemInst->GetCachedASC();
	if (!CAP_ASC || !CAP_ASC->GetGenerics())	return;
	TSubclassOf<UGameplayEffect> DurationBuffGE = CAP_ASC->GetGenerics()->GetItemStatDurationEffect();
	if (!DurationBuffGE)	return;
	if (!ScaleAttribute.IsValid())
	{
		UE_LOG(LogTemp,Warning,TEXT("어떤 스탯에 영향을 받아 감소시킬지 설정해(ScaleAttribute)"));
		return;
	}
	
	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(ItemInst);

	FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(DurationBuffGE, 1.f, Context);
	if (!SpecHandle.IsValid())
		return;
	
	InitGameplayEffectToZero(SpecHandle, DurationBuffGE);
	
	SpecHandle.Data->SetSetByCallerMagnitude(StackTag, TargetStackCount);
	
	float CleanStatValue = ASC->GetNumericAttribute(ScaleAttribute);
	float FinalMagnitude = Magnitude * CleanStatValue;
	
	SpecHandle.Data->SetSetByCallerMagnitude(TargetStatTag, FinalMagnitude*TargetStackCount);

	if (Duration>0.f)
	{
		SpecHandle.Data->SetSetByCallerMagnitude(DurationTag, Duration);
	}
	
	ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

int32 UItemBehavior_ApplyBuffToSelf::GetExistingStackCount(UCAP_ItemInstance* ItemInst, UAbilitySystemComponent* ASC,FActiveGameplayEffectHandle& OutHandle) const
{
	UCAP_AbilitySystemComponent* CAP_ASC = ItemInst->GetCachedASC();
	if (!CAP_ASC || !CAP_ASC->GetGenerics())	return 0;
	TSubclassOf<UGameplayEffect> DurationBuffGE = CAP_ASC->GetGenerics()->GetItemStatDurationEffect();
	if (!DurationBuffGE)	return 0;
	
	int32 FoundStack =0;

	FGameplayEffectQuery Query;
	Query.EffectDefinition = DurationBuffGE;
	TArray<FActiveGameplayEffectHandle> ActiveEffects = ASC->GetActiveEffects(Query);

	for (const FActiveGameplayEffectHandle& Handle : ActiveEffects)
	{
		const FActiveGameplayEffect* ActiveGE = ASC->GetActiveGameplayEffect(Handle);
		// 출처 (SourceObject가 이 효과를 부른 아이템 ItemInst와 동일한지 체크 (다른 인스턴스라면 패스)
		if (ActiveGE && ActiveGE->Spec.GetEffectContext().GetSourceObject() == ItemInst)
		{
			OutHandle = Handle;
			FoundStack = static_cast<int32>(ActiveGE->Spec.GetSetByCallerMagnitude(StackTag,false,0.f));
			break;
		}
	}
	return FoundStack;
}
