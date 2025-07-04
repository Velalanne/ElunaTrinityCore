/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "QuestDef.h"
#include "ConditionMgr.h"
#include "DB2Stores.h"
#include "Field.h"
#include "GameTables.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "QueryResult.h"
#include "QueryResultStructured.h"
#include "QuestPackets.h"
#include "QuestPools.h"
#include "SpellMgr.h"
#include "World.h"
#include "WorldSession.h"

#define QUEST_TEMPLATE_FIELDS (ID)(QuestType)(QuestPackageID)(ContentTuningID)(QuestSortID)(QuestInfoID)(SuggestedGroupNum)(RewardNextQuest)(RewardXPDifficulty)\
    (RewardXPMultiplier)(RewardMoneyDifficulty)(RewardMoneyMultiplier)(RewardBonusMoney)(RewardSpell)(RewardHonor)(RewardKillHonor)(StartItem)\
    (RewardArtifactXPDifficulty)(RewardArtifactXPMultiplier)(RewardArtifactCategoryID)(Flags)(FlagsEx)(FlagsEx2)(FlagsEx3)\
    (RewardItem1)(RewardAmount1)(ItemDrop1)(ItemDropQuantity1)(RewardItem2)(RewardAmount2)(ItemDrop2)(ItemDropQuantity2)\
    (RewardItem3)(RewardAmount3)(ItemDrop3)(ItemDropQuantity3)(RewardItem4)(RewardAmount4)(ItemDrop4)(ItemDropQuantity4)\
    (RewardChoiceItemID1)(RewardChoiceItemQuantity1)(RewardChoiceItemDisplayID1)(RewardChoiceItemID2)(RewardChoiceItemQuantity2)(RewardChoiceItemDisplayID2)\
    (RewardChoiceItemID3)(RewardChoiceItemQuantity3)(RewardChoiceItemDisplayID3)(RewardChoiceItemID4)(RewardChoiceItemQuantity4)(RewardChoiceItemDisplayID4)\
    (RewardChoiceItemID5)(RewardChoiceItemQuantity5)(RewardChoiceItemDisplayID5)(RewardChoiceItemID6)(RewardChoiceItemQuantity6)(RewardChoiceItemDisplayID6)\
    (POIContinent)(POIx)(POIy)(POIPriority)(RewardTitle)(RewardArenaPoints)(RewardSkillLineID)(RewardNumSkillUps)\
    (PortraitGiver)(PortraitGiverMount)(PortraitGiverModelSceneID)(PortraitTurnIn)(RewardFactionID1)(RewardFactionValue1)(RewardFactionOverride1)(RewardFactionCapIn1)\
    (RewardFactionID2)(RewardFactionValue2)(RewardFactionOverride2)(RewardFactionCapIn2)(RewardFactionID3)(RewardFactionValue3)(RewardFactionOverride3)(RewardFactionCapIn3)\
    (RewardFactionID4)(RewardFactionValue4)(RewardFactionOverride4)(RewardFactionCapIn4)(RewardFactionID5)(RewardFactionValue5)(RewardFactionOverride5)(RewardFactionCapIn5)\
    (RewardFactionFlags)(RewardCurrencyID1)(RewardCurrencyQty1)(RewardCurrencyID2)(RewardCurrencyQty2)(RewardCurrencyID3)(RewardCurrencyQty3)\
    (RewardCurrencyID4)(RewardCurrencyQty4)(AcceptedSoundKitID)(CompleteSoundKitID)(AreaGroupID)(TimeAllowed)(AllowableRaces)(ResetByScheduler)(Expansion)\
    (ManagedWorldStateID)(QuestSessionBonus)(LogTitle)(LogDescription)(QuestDescription)(AreaDescription)(PortraitGiverText)(PortraitGiverName)\
    (PortraitTurnInText)(PortraitTurnInName)(QuestCompletionLog)

DEFINE_FIELD_ACCESSOR_CACHE(Quest::, QuestTemplateQueryResult, ResultSet, QUEST_TEMPLATE_FIELDS);

Quest::Quest(QueryResult const& questRecord) : Quest(*questRecord)
{
}

Quest::Quest(QuestTemplateQueryResult const& questRecord) :
    RewardItemId({ questRecord.RewardItem1().GetUInt32(), questRecord.RewardItem2().GetUInt32(),
        questRecord.RewardItem3().GetUInt32(), questRecord.RewardItem4().GetUInt32() }),
    RewardItemCount({ questRecord.RewardAmount1().GetUInt32(), questRecord.RewardAmount2().GetUInt32(),
        questRecord.RewardAmount3().GetUInt32(), questRecord.RewardAmount4().GetUInt32() }),
    ItemDrop({ questRecord.ItemDrop1().GetUInt32(), questRecord.ItemDrop2().GetUInt32(),
        questRecord.ItemDrop3().GetUInt32(), questRecord.ItemDrop4().GetUInt32() }),
    ItemDropQuantity({ questRecord.ItemDropQuantity1().GetUInt32(), questRecord.ItemDropQuantity2().GetUInt32(),
        questRecord.ItemDropQuantity3().GetUInt32(), questRecord.ItemDropQuantity4().GetUInt32() }),
    RewardChoiceItemId({ questRecord.RewardChoiceItemID1().GetUInt32(), questRecord.RewardChoiceItemID2().GetUInt32(),
        questRecord.RewardChoiceItemID3().GetUInt32(), questRecord.RewardChoiceItemID4().GetUInt32(),
        questRecord.RewardChoiceItemID5().GetUInt32(), questRecord.RewardChoiceItemID6().GetUInt32() }),
    RewardChoiceItemCount({ questRecord.RewardChoiceItemQuantity1().GetUInt32(), questRecord.RewardChoiceItemQuantity2().GetUInt32(),
        questRecord.RewardChoiceItemQuantity3().GetUInt32(), questRecord.RewardChoiceItemQuantity4().GetUInt32(),
        questRecord.RewardChoiceItemQuantity5().GetUInt32(), questRecord.RewardChoiceItemQuantity6().GetUInt32() }),
    RewardChoiceItemDisplayId({ questRecord.RewardChoiceItemDisplayID1().GetUInt32(), questRecord.RewardChoiceItemDisplayID2().GetUInt32(),
        questRecord.RewardChoiceItemDisplayID3().GetUInt32(), questRecord.RewardChoiceItemDisplayID4().GetUInt32(),
        questRecord.RewardChoiceItemDisplayID5().GetUInt32(), questRecord.RewardChoiceItemDisplayID6().GetUInt32() }),
    RewardFactionId({ questRecord.RewardFactionID1().GetUInt32(), questRecord.RewardFactionID2().GetUInt32(),
        questRecord.RewardFactionID3().GetUInt32(), questRecord.RewardFactionID4().GetUInt32(), questRecord.RewardFactionID5().GetUInt32() }),
    RewardFactionValue({ questRecord.RewardFactionValue1().GetInt32(), questRecord.RewardFactionValue2().GetInt32(),
        questRecord.RewardFactionValue3().GetInt32(), questRecord.RewardFactionValue4().GetInt32(), questRecord.RewardFactionValue5().GetInt32() }),
    RewardFactionOverride({ questRecord.RewardFactionOverride1().GetInt32(), questRecord.RewardFactionOverride2().GetInt32(),
        questRecord.RewardFactionOverride3().GetInt32(), questRecord.RewardFactionOverride4().GetInt32(), questRecord.RewardFactionOverride5().GetInt32() }),
    RewardFactionCapIn({ questRecord.RewardFactionCapIn1().GetInt32(), questRecord.RewardFactionCapIn2().GetInt32(),
        questRecord.RewardFactionCapIn3().GetInt32(), questRecord.RewardFactionCapIn4().GetInt32(), questRecord.RewardFactionCapIn5().GetInt32() }),
    RewardCurrencyId({ questRecord.RewardCurrencyID1().GetUInt32(), questRecord.RewardCurrencyID2().GetUInt32(),
        questRecord.RewardCurrencyID3().GetUInt32(), questRecord.RewardCurrencyID4().GetUInt32() }),
    RewardCurrencyCount({ questRecord.RewardCurrencyQty1().GetUInt32(), questRecord.RewardCurrencyQty2().GetUInt32(),
        questRecord.RewardCurrencyQty3().GetUInt32(), questRecord.RewardCurrencyQty4().GetUInt32() }),
    _rewItemsCount(std::ranges::count_if(RewardItemId, [](uint32 itemId) { return itemId != 0; })),
    _rewChoiceItemsCount(std::ranges::count_if(RewardChoiceItemId, [](uint32 itemId) { return itemId != 0; })),
    _id(questRecord.ID().GetUInt32()),
    _type(questRecord.QuestType().GetUInt8()),
    _packageID(questRecord.QuestPackageID().GetUInt32()),
    _contentTuningID(questRecord.ContentTuningID().GetInt32()),
    _questSortID(questRecord.QuestSortID().GetInt16()),
    _questInfoID(questRecord.QuestInfoID().GetUInt16()),
    _suggestedPlayers(questRecord.SuggestedGroupNum().GetUInt8()),
    _nextQuestInChain(questRecord.RewardNextQuest().GetUInt32()),
    _rewardXPDifficulty(questRecord.RewardXPDifficulty().GetUInt32()),
    _rewardXPMultiplier(questRecord.RewardXPMultiplier().GetFloat()),
    _rewardMoneyDifficulty(questRecord.RewardMoneyDifficulty().GetUInt32()),
    _rewardMoneyMultiplier(questRecord.RewardMoneyMultiplier().GetFloat()),
    _rewardBonusMoney(questRecord.RewardBonusMoney().GetUInt32()),
    _rewardSpell(questRecord.RewardSpell().GetUInt32()),
    _rewardHonor(questRecord.RewardHonor().GetUInt32()),
    _rewardKillHonor(questRecord.RewardKillHonor().GetUInt32()),
    _rewardArtifactXPDifficulty(questRecord.RewardArtifactXPDifficulty().GetUInt32()),
    _rewardArtifactXPMultiplier(questRecord.RewardArtifactXPMultiplier().GetFloat()),
    _rewardArtifactCategoryID(questRecord.RewardArtifactCategoryID().GetUInt32()),
    _sourceItemId(questRecord.StartItem().GetUInt32()),
    _flags(questRecord.Flags().GetUInt32()),
    _flagsEx(questRecord.FlagsEx().GetUInt32()),
    _flagsEx2(questRecord.FlagsEx2().GetUInt32()),
    _flagsEx3(questRecord.FlagsEx3().GetUInt32()),
    _poiContinent(questRecord.POIContinent().GetUInt32()),
    _poix(questRecord.POIx().GetFloat()),
    _poiy(questRecord.POIy().GetFloat()),
    _poiPriority(questRecord.POIPriority().GetUInt32()),
    _rewardTitleId(questRecord.RewardTitle().GetUInt32()),
    _rewardArenaPoints(questRecord.RewardArenaPoints().GetUInt32()),
    _rewardSkillId(questRecord.RewardSkillLineID().GetUInt32()),
    _rewardSkillPoints(questRecord.RewardNumSkillUps().GetUInt32()),
    _questGiverPortrait(questRecord.PortraitGiver().GetUInt32()),
    _questGiverPortraitMount(questRecord.PortraitGiverMount().GetUInt32()),
    _questGiverPortraitModelSceneId(questRecord.PortraitGiverModelSceneID().GetInt32()),
    _questTurnInPortrait(questRecord.PortraitTurnIn().GetUInt32()),
    _rewardReputationMask(questRecord.RewardFactionFlags().GetUInt32()),
    _soundAccept(questRecord.AcceptedSoundKitID().GetUInt32()),
    _soundTurnIn(questRecord.CompleteSoundKitID().GetUInt32()),
    _areaGroupID(questRecord.AreaGroupID().GetUInt32()),
    _limitTime(questRecord.TimeAllowed().GetInt64()),
    _allowableRaces({ .RawValue = questRecord.AllowableRaces().GetUInt64() }),
    _expansion(questRecord.Expansion().GetInt32()),
    _managedWorldStateID(questRecord.ManagedWorldStateID().GetInt32()),
    _questSessionBonus(questRecord.QuestSessionBonus().GetInt32()),
    _logTitle(questRecord.LogTitle().GetStringView()),
    _logDescription(questRecord.LogDescription().GetStringView()),
    _questDescription(questRecord.QuestDescription().GetStringView()),
    _areaDescription(questRecord.AreaDescription().GetStringView()),
    _portraitGiverText(questRecord.PortraitGiverText().GetStringView()),
    _portraitGiverName(questRecord.PortraitGiverName().GetStringView()),
    _portraitTurnInText(questRecord.PortraitTurnInText().GetStringView()),
    _portraitTurnInName(questRecord.PortraitTurnInName().GetStringView()),
    _questCompletionLog(questRecord.QuestCompletionLog().GetStringView()),
    _resetByScheduler(questRecord.ResetByScheduler().GetBool())
{
}

Quest::~Quest()
{
    for (QuestObjective& objective : Objectives)
        delete objective.CompletionEffect;
}

void Quest::LoadRewardDisplaySpell(Field* fields)
{
    uint32 spellId = fields[1].GetUInt32();
    uint32 playerConditionId = fields[2].GetUInt32();
    uint32 type = fields[3].GetUInt32();

    if (!sSpellMgr->GetSpellInfo(spellId, DIFFICULTY_NONE))
    {
        TC_LOG_ERROR("sql.sql", "Table `quest_reward_display_spell` has non-existing Spell ({}) set for quest {}. Skipped.", spellId, fields[0].GetUInt32());
        return;
    }

    if (playerConditionId && !sPlayerConditionStore.LookupEntry(playerConditionId))
    {
        if (!sConditionMgr->HasConditionsForNotGroupedEntry(CONDITION_SOURCE_TYPE_PLAYER_CONDITION, playerConditionId))
        {
            TC_LOG_ERROR("sql.sql", "Table `quest_reward_display_spell` has serverside PlayerCondition ({}) set for quest {} and spell {} without conditions. Set to 0.", playerConditionId, fields[0].GetUInt32(), spellId);
            playerConditionId = 0;
        }
    }

    if (type >= AsUnderlyingType(QuestCompleteSpellType::Max))
    {
        TC_LOG_ERROR("sql.sql", "Table `quest_reward_display_spell` invalid type value ({}) set for quest {} and spell {}. Set to 0.", type, fields[0].GetUInt32(), spellId);
        type = AsUnderlyingType(QuestCompleteSpellType::LegacyBehavior);
    }

    RewardDisplaySpell.emplace_back(spellId, playerConditionId, QuestCompleteSpellType(type));
}

void Quest::LoadRewardChoiceItems(Field* fields)
{
    for (uint32 i = 0; i < QUEST_REWARD_CHOICES_COUNT; ++i)
        RewardChoiceItemType[i] = LootItemType(fields[1 + i].GetUInt8());
}

void Quest::LoadQuestDetails(Field* fields)
{
    for (uint32 i = 0; i < QUEST_EMOTE_COUNT; ++i)
    {
        if (!sEmotesStore.LookupEntry(fields[1 + i].GetUInt16()))
        {
            TC_LOG_ERROR("sql.sql", "Table `quest_details` has non-existing Emote{} ({}) set for quest {}. Skipped.", 1+i, fields[1+i].GetUInt16(), fields[0].GetUInt32());
            continue;
        }

        DetailsEmote[i] = fields[1 + i].GetUInt16();
    }

    for (uint32 i = 0; i < QUEST_EMOTE_COUNT; ++i)
        DetailsEmoteDelay[i] = fields[5 + i].GetUInt32();
}

void Quest::LoadQuestRequestItems(Field* fields)
{
    _emoteOnComplete = fields[1].GetUInt16();
    _emoteOnIncomplete = fields[2].GetUInt16();

    if (!sEmotesStore.LookupEntry(_emoteOnComplete))
        TC_LOG_ERROR("sql.sql", "Table `quest_request_items` has non-existing EmoteOnComplete ({}) set for quest {}.", _emoteOnComplete, fields[0].GetUInt32());

    if (!sEmotesStore.LookupEntry(_emoteOnIncomplete))
        TC_LOG_ERROR("sql.sql", "Table `quest_request_items` has non-existing EmoteOnIncomplete ({}) set for quest {}.", _emoteOnIncomplete, fields[0].GetUInt32());

    _emoteOnCompleteDelay = fields[3].GetUInt32();
    _emoteOnIncompleteDelay = fields[4].GetUInt32();
    _requestItemsText = fields[5].GetStringView();
}

void Quest::LoadQuestOfferReward(Field* fields)
{
    for (uint32 i = 0; i < QUEST_EMOTE_COUNT; ++i)
    {
        if (!sEmotesStore.LookupEntry(fields[1 + i].GetUInt16()))
        {
            TC_LOG_ERROR("sql.sql", "Table `quest_offer_reward` has non-existing Emote{} ({}) set for quest {}. Skipped.", 1+i, fields[1+i].GetUInt16(), fields[0].GetUInt32());
            continue;
        }

        OfferRewardEmote[i] = fields[1 + i].GetInt16();
    }

    for (uint32 i = 0; i < QUEST_EMOTE_COUNT; ++i)
        OfferRewardEmoteDelay[i] = fields[5 + i].GetUInt32();

    _offerRewardText = fields[9].GetStringView();
}

void Quest::LoadQuestTemplateAddon(Field* fields)
{
    _maxLevel = fields[1].GetUInt8();
    _allowableClasses = fields[2].GetUInt32();
    _sourceSpellID = fields[3].GetUInt32();
    _prevQuestID = fields[4].GetInt32();
    _nextQuestID = fields[5].GetUInt32();
    _exclusiveGroup = fields[6].GetInt32();
    _breadcrumbForQuestId = fields[7].GetInt32();
    _rewardMailTemplateId = fields[8].GetUInt32();
    _rewardMailDelay = fields[9].GetUInt32();
    _requiredSkillId = fields[10].GetUInt16();
    _requiredSkillPoints = fields[11].GetUInt16();
    _requiredMinRepFaction = fields[12].GetUInt16();
    _requiredMaxRepFaction = fields[13].GetUInt16();
    _requiredMinRepValue = fields[14].GetInt32();
    _requiredMaxRepValue = fields[15].GetInt32();
    _sourceItemIdCount = fields[16].GetUInt8();
    _specialFlags = fields[17].GetUInt8();
    _scriptId = sObjectMgr->GetScriptId(fields[18].GetString());

    if (_specialFlags & QUEST_SPECIAL_FLAGS_AUTO_ACCEPT)
        _flags |= QUEST_FLAGS_AUTO_ACCEPT;
}

void Quest::LoadQuestMailSender(Field* fields)
{
    _rewardMailSenderEntry = fields[1].GetUInt32();
}

void Quest::LoadQuestObjective(Field* fields)
{
    QuestObjective& obj = Objectives.emplace_back();
    obj.QuestID = fields[0].GetUInt32();
    obj.ID = fields[1].GetUInt32();
    obj.Type = fields[2].GetUInt8();
    obj.StorageIndex = fields[3].GetInt8();
    obj.ObjectID = fields[4].GetInt32();
    obj.Amount = fields[5].GetInt32();
    obj.Flags = fields[6].GetUInt32();
    obj.Flags2 = fields[7].GetUInt32();
    obj.ProgressBarWeight = fields[8].GetFloat();
    obj.Description = fields[9].GetStringView();

    bool hasCompletionEffect = std::any_of(fields + 10, fields + 15, [](Field const& f) { return !f.IsNull(); });
    if (hasCompletionEffect)
    {
        obj.CompletionEffect = new QuestObjectiveAction();
        obj.CompletionEffect->GameEventId = fields[10].GetUInt32OrNull();
        obj.CompletionEffect->SpellId = fields[11].GetUInt32OrNull();
        obj.CompletionEffect->ConversationId = fields[12].GetUInt32OrNull();
        obj.CompletionEffect->UpdatePhaseShift = fields[13].GetBool();
        obj.CompletionEffect->UpdateZoneAuras = fields[14].GetBool();
    }

    _usedQuestObjectiveTypes[obj.Type] = true;
}

void Quest::LoadQuestObjectiveVisualEffect(Field* fields)
{
    uint32 objID = fields[1].GetUInt32();

    for (QuestObjective& obj : Objectives)
    {
        if (obj.ID == objID)
        {
            uint8 effectIndex = fields[3].GetUInt8();
            if (effectIndex >= obj.VisualEffects.size())
                obj.VisualEffects.resize(effectIndex + 1, 0);

            obj.VisualEffects[effectIndex] = fields[4].GetInt32();
            break;
        }
    }
}

void Quest::LoadConditionalConditionalQuestDescription(Field* fields)
{
    LocaleConstant locale = GetLocaleByName(fields[4].GetStringView());
    if (!sWorld->getBoolConfig(CONFIG_LOAD_LOCALES) && locale != DEFAULT_LOCALE)
        return;

    if (locale >= TOTAL_LOCALES)
    {
        TC_LOG_ERROR("sql.sql", "Table `quest_description_conditional` has invalid locale {} set for quest {}. Skipped.", fields[4].GetCString(), fields[0].GetUInt32());
        return;
    }

    auto itr = std::find_if(_conditionalQuestDescription.begin(), _conditionalQuestDescription.end(), [fields](QuestConditionalText const& text)
    {
        return text.PlayerConditionId == fields[1].GetInt32() && text.QuestgiverCreatureId == fields[2].GetInt32();
    });

    QuestConditionalText& text = itr != _conditionalQuestDescription.end() ? *itr : _conditionalQuestDescription.emplace_back();
    text.PlayerConditionId = fields[1].GetInt32();
    text.QuestgiverCreatureId = fields[2].GetInt32();
    ObjectMgr::AddLocaleString(fields[3].GetStringView(), locale, text.Text);
}

void Quest::LoadConditionalConditionalRequestItemsText(Field* fields)
{
    LocaleConstant locale = GetLocaleByName(fields[4].GetStringView());
    if (!sWorld->getBoolConfig(CONFIG_LOAD_LOCALES) && locale != DEFAULT_LOCALE)
        return;

    if (locale >= TOTAL_LOCALES)
    {
        TC_LOG_ERROR("sql.sql", "Table `quest_request_items_conditional` has invalid locale {} set for quest {}. Skipped.", fields[4].GetCString(), fields[0].GetUInt32());
        return;
    }

    auto itr = std::find_if(_conditionalRequestItemsText.begin(), _conditionalRequestItemsText.end(), [fields](QuestConditionalText const& text)
    {
        return text.PlayerConditionId == fields[1].GetInt32() && text.QuestgiverCreatureId == fields[2].GetInt32();
    });

    QuestConditionalText& text = itr != _conditionalRequestItemsText.end() ? *itr : _conditionalRequestItemsText.emplace_back();
    text.PlayerConditionId = fields[1].GetInt32();
    text.QuestgiverCreatureId = fields[2].GetInt32();
    ObjectMgr::AddLocaleString(fields[3].GetStringView(), locale, text.Text);
}

void Quest::LoadConditionalConditionalOfferRewardText(Field* fields)
{
    LocaleConstant locale = GetLocaleByName(fields[4].GetStringView());
    if (!sWorld->getBoolConfig(CONFIG_LOAD_LOCALES) && locale != DEFAULT_LOCALE)
        return;

    if (locale >= TOTAL_LOCALES)
    {
        TC_LOG_ERROR("sql.sql", "Table `quest_offer_reward_conditional` has invalid locale {} set for quest {}. Skipped.", fields[4].GetCString(), fields[0].GetUInt32());
        return;
    }

    auto itr = std::find_if(_conditionalOfferRewardText.begin(), _conditionalOfferRewardText.end(), [fields](QuestConditionalText const& text)
    {
        return text.PlayerConditionId == fields[1].GetInt32() && text.QuestgiverCreatureId == fields[2].GetInt32();
    });

    QuestConditionalText& text = itr != _conditionalOfferRewardText.end() ? *itr : _conditionalOfferRewardText.emplace_back();
    text.PlayerConditionId = fields[1].GetInt32();
    text.QuestgiverCreatureId = fields[2].GetInt32();
    ObjectMgr::AddLocaleString(fields[3].GetStringView(), locale, text.Text);
}

void Quest::LoadConditionalConditionalQuestCompletionLog(Field* fields)
{
    LocaleConstant locale = GetLocaleByName(fields[4].GetStringView());
    if (!sWorld->getBoolConfig(CONFIG_LOAD_LOCALES) && locale != DEFAULT_LOCALE)
        return;

    if (locale >= TOTAL_LOCALES)
    {
        TC_LOG_ERROR("sql.sql", "Table `quest_completion_log_conditional` has invalid locale {} set for quest {}. Skipped.", fields[4].GetCString(), fields[0].GetUInt32());
        return;
    }

    auto itr = std::find_if(_conditionalQuestCompletionLog.begin(), _conditionalQuestCompletionLog.end(), [fields](QuestConditionalText const& text)
    {
        return text.PlayerConditionId == fields[1].GetInt32() && text.QuestgiverCreatureId == fields[2].GetInt32();
    });

    QuestConditionalText& text = itr != _conditionalQuestCompletionLog.end() ? *itr : _conditionalQuestCompletionLog.emplace_back();
    text.PlayerConditionId = fields[1].GetInt32();
    text.QuestgiverCreatureId = fields[2].GetInt32();
    ObjectMgr::AddLocaleString(fields[3].GetStringView(), locale, text.Text);
}

void Quest::LoadTreasurePickers(Field* fields)
{
    _treasurePickerID.push_back(fields[1].GetInt32());
}

uint32 Quest::XPValue(Player const* player) const
{
    return XPValue(player, GetContentTuningId(), _rewardXPDifficulty, _rewardXPMultiplier, _expansion);
}

uint32 Quest::XPValue(Player const* player, uint32 contentTuningId, uint32 xpDifficulty, float xpMultiplier /*= 1.0f*/, int32 expansion /*= -1*/)
{
    if (player)
    {
        uint32 questLevel = player->GetQuestLevel(contentTuningId);
        QuestXPEntry const* questXp = sQuestXPStore.LookupEntry(questLevel);
        if (!questXp || xpDifficulty >= 10)
            return 0;

        uint32 xp = questXp->Difficulty[xpDifficulty];
        if (ContentTuningEntry const* contentTuning = sContentTuningStore.LookupEntry(contentTuningId))
            xp = xp * contentTuning->QuestXpMultiplier;

        int32 diffFactor = 2 * (questLevel - player->GetLevel()) + 12;
        if (diffFactor < 1)
            diffFactor = 1;
        else if (diffFactor > 10)
            diffFactor = 10;

        xp = diffFactor * xp * xpMultiplier / 10;
        if (player->GetLevel() >= GetMaxLevelForExpansion(CURRENT_EXPANSION - 1) && player->GetSession()->GetExpansion() == CURRENT_EXPANSION && expansion >= 0 && expansion < CURRENT_EXPANSION)
            xp = uint32(xp / 9.0f);

        xp = RoundXPValue(xp);

        if (sWorld->getIntConfig(CONFIG_MIN_QUEST_SCALED_XP_RATIO))
        {
            uint32 minScaledXP = RoundXPValue(questXp->Difficulty[xpDifficulty] * xpMultiplier) * sWorld->getIntConfig(CONFIG_MIN_QUEST_SCALED_XP_RATIO) / 100;
            xp = std::max(minScaledXP, xp);
        }

        return xp;
    }

    return 0;
}

/*static*/ bool Quest::IsTakingQuestEnabled(uint32 questId)
{
    if (!sQuestPoolMgr->IsQuestActive(questId))
        return false;

    return true;
}

uint32 Quest::MoneyValue(Player const* player) const
{
    if (QuestMoneyRewardEntry const* money = sQuestMoneyRewardStore.LookupEntry(player->GetQuestLevel(this)))
        return money->Difficulty[GetRewMoneyDifficulty()] * GetMoneyMultiplier();
    else
        return 0;
}

uint32 Quest::MaxMoneyValue() const
{
    uint32 value = 0;
    if (Optional<ContentTuningLevels> questLevels = sDB2Manager.GetContentTuningData(GetContentTuningId(), 0))
        if (QuestMoneyRewardEntry const* money = sQuestMoneyRewardStore.LookupEntry(questLevels->MaxLevel))
            value = money->Difficulty[GetRewMoneyDifficulty()] * GetMoneyMultiplier();

    return value;
}

uint32 Quest::GetMaxMoneyReward() const
{
    return MaxMoneyValue() * sWorld->getRate(RATE_MONEY_QUEST);
}

Optional<QuestTagType> Quest::GetQuestTag() const
{
    if (QuestInfoEntry const* questInfo = sQuestInfoStore.LookupEntry(GetQuestInfoID()))
        return static_cast<QuestTagType>(questInfo->Type);

    return {};
}

bool Quest::IsImportant() const
{
    if (QuestInfoEntry const* questInfo = sQuestInfoStore.LookupEntry(GetQuestInfoID()))
        return (questInfo->Modifiers & 0x400) != 0;

    return false;
}

bool Quest::IsMeta() const
{
    if (QuestInfoEntry const* questInfo = sQuestInfoStore.LookupEntry(GetQuestInfoID()))
        return (questInfo->Modifiers & 0x800) != 0;

    return false;
}

void Quest::BuildQuestRewards(WorldPackets::Quest::QuestRewards& rewards, Player* player) const
{
    rewards.ChoiceItemCount         = GetRewChoiceItemsCount();
    rewards.ItemCount               = GetRewItemsCount();
    rewards.Money                   = player->GetQuestMoneyReward(this);
    rewards.XP                      = player->GetQuestXPReward(this);
    rewards.ArtifactCategoryID      = GetArtifactCategoryId();
    rewards.Title                   = GetRewTitle();
    rewards.FactionFlags            = GetRewardReputationMask();
    auto displaySpellItr = rewards.SpellCompletionDisplayID.begin();
    for (QuestRewardDisplaySpell displaySpell : RewardDisplaySpell)
    {
        if (!ConditionMgr::IsPlayerMeetingCondition(player, displaySpell.PlayerConditionId))
            continue;

        *displaySpellItr = displaySpell.SpellId;
        if (++displaySpellItr == rewards.SpellCompletionDisplayID.end())
            break;
    }

    rewards.SpellCompletionID       = GetRewSpell();
    rewards.SkillLineID             = GetRewardSkillId();
    rewards.NumSkillUps             = GetRewardSkillPoints();
    rewards.TreasurePickerID        = GetTreasurePickerId();

    for (uint32 i = 0; i < QUEST_REWARD_CHOICES_COUNT; ++i)
    {
        rewards.ChoiceItems[i].LootItemType = RewardChoiceItemType[i];
        rewards.ChoiceItems[i].Item.ItemID = RewardChoiceItemId[i];
        rewards.ChoiceItems[i].Quantity = RewardChoiceItemCount[i];
    }

    for (uint32 i = 0; i < QUEST_REWARD_ITEM_COUNT; ++i)
    {
        rewards.Items[i].ItemID = RewardItemId[i];
        rewards.Items[i].ItemQty = RewardItemCount[i];
    }

    for (uint32 i = 0; i < QUEST_REWARD_REPUTATIONS_COUNT; ++i)
    {
        rewards.FactionID[i] = RewardFactionId[i];
        rewards.FactionValue[i] = RewardFactionValue[i];
        rewards.FactionOverride[i] = RewardFactionOverride[i];
        rewards.FactionCapIn[i] = RewardFactionCapIn[i];
    }

    for (uint32 i = 0; i < QUEST_REWARD_CURRENCY_COUNT; ++i)
    {
        rewards.Currencies[i].CurrencyID = RewardCurrencyId[i];
        rewards.Currencies[i].CurrencyQty = RewardCurrencyCount[i];
    }
}

uint32 Quest::GetRewMoneyMaxLevel() const
{
    // If Quest has flag to not give money on max level, it's 0
    if (HasFlag(QUEST_FLAGS_NO_MONEY_FOR_XP))
        return 0;

    // Else, return the rewarded copper sum modified by the rate
    return uint32(_rewardBonusMoney * sWorld->getRate(RATE_MONEY_MAX_LEVEL_QUEST));
}

bool Quest::IsAutoAccept() const
{
    return !sWorld->getBoolConfig(CONFIG_QUEST_IGNORE_AUTO_ACCEPT) && HasFlag(QUEST_FLAGS_AUTO_ACCEPT);
}

bool Quest::IsTurnIn() const
{
    return !sWorld->getBoolConfig(CONFIG_QUEST_IGNORE_AUTO_COMPLETE) && _type == QUEST_TYPE_TURNIN;
}

bool Quest::IsRaidQuest(Difficulty difficulty) const
{
    switch (_questInfoID)
    {
        case QUEST_INFO_RAID:
            return true;
        case QUEST_INFO_RAID_10:
            return difficulty == DIFFICULTY_10_N || difficulty == DIFFICULTY_10_HC;
        case QUEST_INFO_RAID_25:
            return difficulty == DIFFICULTY_25_N || difficulty == DIFFICULTY_25_HC;
        default:
            break;
    }

    if ((_flags & QUEST_FLAGS_RAID_GROUP_OK) != 0)
        return true;

    return false;
}

bool Quest::IsAllowedInRaid(Difficulty difficulty) const
{
    if (IsRaidQuest(difficulty))
        return true;

    return sWorld->getBoolConfig(CONFIG_QUEST_IGNORE_RAID);
}

uint32 Quest::CalculateHonorGain(uint8 /*level*/) const
{
    uint32 honor = 0;

    /*if (GetRewHonorAddition() > 0 || GetRewHonorMultiplier() > 0.0f)
    {
        // values stored from 0.. for 1...
        TeamContributionPointsEntry const* tc = sTeamContributionPointsStore.LookupEntry(level);
        if (!tc)
            return 0;

        honor = uint32(tc->Data * GetRewHonorMultiplier() * 0.1f);
        honor += GetRewHonorAddition();
    }*/

    return honor;
}

bool Quest::CanIncreaseRewardedQuestCounters() const
{
    // Dungeon Finder/Daily/Repeatable (if not weekly, monthly or seasonal) quests are never considered rewarded serverside.
    // This affects counters and client requests for completed quests.
    return (!IsDFQuest() && !IsDaily() && (!IsRepeatable() || IsWeekly() || IsMonthly() || IsSeasonal()));
}

void Quest::InitializeQueryData()
{
    for (uint8 loc = LOCALE_enUS; loc < TOTAL_LOCALES; ++loc)
    {
        if (!sWorld->getBoolConfig(CONFIG_LOAD_LOCALES) && loc != DEFAULT_LOCALE)
            continue;

        QueryData[loc] = BuildQueryData(static_cast<LocaleConstant>(loc), nullptr);
    }
}

WorldPacket Quest::BuildQueryData(LocaleConstant loc, Player* player) const
{
    WorldPackets::Quest::QueryQuestInfoResponse response;

    response.Allow = true;
    response.QuestID = GetQuestId();

    response.Info.LogTitle = GetLogTitle();
    response.Info.LogDescription = GetLogDescription();
    response.Info.QuestDescription = GetQuestDescription();
    response.Info.AreaDescription = GetAreaDescription();
    response.Info.QuestCompletionLog = GetQuestCompletionLog();
    response.Info.PortraitGiverText = GetPortraitGiverText();
    response.Info.PortraitGiverName = GetPortraitGiverName();
    response.Info.PortraitTurnInText = GetPortraitTurnInText();
    response.Info.PortraitTurnInName = GetPortraitTurnInName();
    std::transform(GetConditionalQuestDescription().begin(), GetConditionalQuestDescription().end(), std::back_inserter(response.Info.ConditionalQuestDescription), [loc](QuestConditionalText const& text)
    {
        std::string_view content = text.Text[LOCALE_enUS];
        ObjectMgr::GetLocaleString(text.Text, loc, content);
        return WorldPackets::Quest::ConditionalQuestText { text.PlayerConditionId, text.QuestgiverCreatureId, content };
    });

    std::transform(GetConditionalQuestCompletionLog().begin(), GetConditionalQuestCompletionLog().end(), std::back_inserter(response.Info.ConditionalQuestCompletionLog), [loc](QuestConditionalText const& text)
    {
        std::string_view content = text.Text[LOCALE_enUS];
        ObjectMgr::GetLocaleString(text.Text, loc, content);
        return WorldPackets::Quest::ConditionalQuestText { text.PlayerConditionId, text.QuestgiverCreatureId, content };
    });

    if (loc != LOCALE_enUS)
    {
        if (QuestTemplateLocale const* questTemplateLocale = sObjectMgr->GetQuestLocale(GetQuestId()))
        {
            ObjectMgr::GetLocaleString(questTemplateLocale->LogTitle,           loc, response.Info.LogTitle);
            ObjectMgr::GetLocaleString(questTemplateLocale->LogDescription,     loc, response.Info.LogDescription);
            ObjectMgr::GetLocaleString(questTemplateLocale->QuestDescription,   loc, response.Info.QuestDescription);
            ObjectMgr::GetLocaleString(questTemplateLocale->AreaDescription,    loc, response.Info.AreaDescription);
            ObjectMgr::GetLocaleString(questTemplateLocale->QuestCompletionLog, loc, response.Info.QuestCompletionLog);
            ObjectMgr::GetLocaleString(questTemplateLocale->PortraitGiverText,  loc, response.Info.PortraitGiverText);
            ObjectMgr::GetLocaleString(questTemplateLocale->PortraitGiverName,  loc, response.Info.PortraitGiverName);
            ObjectMgr::GetLocaleString(questTemplateLocale->PortraitTurnInText, loc, response.Info.PortraitTurnInText);
            ObjectMgr::GetLocaleString(questTemplateLocale->PortraitTurnInName, loc, response.Info.PortraitTurnInName);
        }
    }

    response.Info.QuestID = GetQuestId();
    response.Info.QuestType = GetQuestType();
    response.Info.ContentTuningID = GetContentTuningId();
    response.Info.QuestPackageID = GetQuestPackageID();
    response.Info.QuestSortID = GetZoneOrSort();
    response.Info.QuestInfoID = GetQuestInfoID();
    response.Info.SuggestedGroupNum = GetSuggestedPlayers();
    response.Info.RewardNextQuest = GetNextQuestInChain();
    response.Info.RewardXPDifficulty = GetXPDifficulty();
    response.Info.RewardXPMultiplier = GetXPMultiplier();

    if (!HasFlag(QUEST_FLAGS_HIDE_REWARD))
        response.Info.RewardMoney = player ? player->GetQuestMoneyReward(this) : GetMaxMoneyReward();

    response.Info.RewardMoneyDifficulty = GetRewMoneyDifficulty();
    response.Info.RewardMoneyMultiplier = GetMoneyMultiplier();
    response.Info.RewardBonusMoney = GetRewMoneyMaxLevel();
    for (QuestRewardDisplaySpell displaySpell : RewardDisplaySpell)
    {
        WorldPackets::Quest::QuestCompleteDisplaySpell& rewardDisplaySpell = response.Info.RewardDisplaySpell.emplace_back();
        rewardDisplaySpell.SpellID = displaySpell.SpellId;
        rewardDisplaySpell.PlayerConditionID = displaySpell.PlayerConditionId;
        rewardDisplaySpell.Type = int32(displaySpell.Type);
    }

    response.Info.RewardSpell = GetRewSpell();

    response.Info.RewardHonor = GetRewHonor();
    response.Info.RewardKillHonor = GetRewKillHonor();

    response.Info.RewardArtifactXPDifficulty = GetArtifactXPDifficulty();
    response.Info.RewardArtifactXPMultiplier = GetArtifactXPMultiplier();
    response.Info.RewardArtifactCategoryID = GetArtifactCategoryId();

    response.Info.StartItem = GetSrcItemId();
    response.Info.Flags = GetFlags();
    response.Info.FlagsEx = GetFlagsEx();
    response.Info.FlagsEx2 = GetFlagsEx2();
    response.Info.FlagsEx3 = GetFlagsEx3();
    response.Info.RewardTitle = GetRewTitle();
    response.Info.RewardArenaPoints = GetRewArenaPoints();
    response.Info.RewardSkillLineID = GetRewardSkillId();
    response.Info.RewardNumSkillUps = GetRewardSkillPoints();
    response.Info.RewardFactionFlags = GetRewardReputationMask();
    response.Info.PortraitGiver = GetQuestGiverPortrait();
    response.Info.PortraitGiverMount = GetQuestGiverPortraitMount();
    response.Info.PortraitGiverModelSceneID = GetQuestGiverPortraitModelSceneId();
    response.Info.PortraitTurnIn = GetQuestTurnInPortrait();

    for (uint8 i = 0; i < QUEST_ITEM_DROP_COUNT; ++i)
    {
        response.Info.ItemDrop[i] = ItemDrop[i];
        response.Info.ItemDropQuantity[i] = ItemDropQuantity[i];
    }

    if (!HasFlag(QUEST_FLAGS_HIDE_REWARD))
    {
        for (uint8 i = 0; i < QUEST_REWARD_ITEM_COUNT; ++i)
        {
            response.Info.RewardItems[i] = RewardItemId[i];
            response.Info.RewardAmount[i] = RewardItemCount[i];
        }
        for (uint8 i = 0; i < QUEST_REWARD_CHOICES_COUNT; ++i)
        {
            response.Info.UnfilteredChoiceItems[i].ItemID = RewardChoiceItemId[i];
            response.Info.UnfilteredChoiceItems[i].Quantity = RewardChoiceItemCount[i];
        }
    }

    for (uint8 i = 0; i < QUEST_REWARD_REPUTATIONS_COUNT; ++i)
    {
        response.Info.RewardFactionID[i] = RewardFactionId[i];
        response.Info.RewardFactionValue[i] = RewardFactionValue[i];
        response.Info.RewardFactionOverride[i] = RewardFactionOverride[i];
        response.Info.RewardFactionCapIn[i] = RewardFactionCapIn[i];
    }

    response.Info.POIContinent = GetPOIContinent();
    response.Info.POIx = GetPOIx();
    response.Info.POIy = GetPOIy();
    response.Info.POIPriority = GetPOIPriority();

    response.Info.AllowableRaces = GetAllowableRaces();
    response.Info.TreasurePickerID = GetTreasurePickerId();
    response.Info.Expansion = GetExpansion();
    response.Info.ManagedWorldStateID = GetManagedWorldStateId();
    response.Info.QuestSessionBonus = 0; //GetQuestSessionBonus(); // this is only sent while quest session is active
    response.Info.QuestGiverCreatureID = 0; // only sent during npc interaction

    for (QuestObjective const& questObjective : GetObjectives())
    {
        response.Info.Objectives.push_back(questObjective);

        if (loc != LOCALE_enUS)
        {
            if (QuestObjectivesLocale const* questObjectivesLocale = sObjectMgr->GetQuestObjectivesLocale(questObjective.ID))
                ObjectMgr::GetLocaleString(questObjectivesLocale->Description, loc, response.Info.Objectives.back().Description);
        }
    }

    for (uint32 i = 0; i < QUEST_REWARD_CURRENCY_COUNT; ++i)
    {
        response.Info.RewardCurrencyID[i] = RewardCurrencyId[i];
        response.Info.RewardCurrencyQty[i] = RewardCurrencyCount[i];
    }

    response.Info.AcceptedSoundKitID = GetSoundAccept();
    response.Info.CompleteSoundKitID = GetSoundTurnIn();
    response.Info.AreaGroupID = GetAreaGroupID();
    response.Info.TimeAllowed = GetLimitTime();
    response.Info.ResetByScheduler = IsResetByScheduler();

    response.Write();
    response.ShrinkToFit();
    return response.Move();
}

uint32 Quest::RoundXPValue(uint32 xp)
{
    if (xp <= 100)
        return 5 * ((xp + 2) / 5);
    else if (xp <= 500)
        return 10 * ((xp + 5) / 10);
    else if (xp <= 1000)
        return 25 * ((xp + 12) / 25);
    else
        return 50 * ((xp + 25) / 50);
}
