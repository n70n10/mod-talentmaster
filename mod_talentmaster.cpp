#include "Player.h"
#include "ScriptMgr.h"
#include "Config.h"
#include "Chat.h"
#include "ChatCommand.h"
#include "GossipDef.h"
#include "ObjectMgr.h"
#include "Log.h"
#include "Pet.h"
#include "ScriptedGossip.h"

using namespace Acore::ChatCommands;

// ============================================================
//  mod-talentmaster
//
//  Provides free, unrestricted talent resets to regular players
//  through two optional interfaces, each independently toggled
//  in the config:
//
//    1. CommandScript  – player chat command:
//         .talent reset          reset all talents for free
//         .talent resetpet       reset pet talents for free (Hunter)
//
//    2. GossipScript   – a custom in-world NPC players can talk to.
//         The NPC entry ID is set in the config so you can attach
//         this behaviour to any existing or custom creature.
// ============================================================

// ============================================================
//  CommandScript
// ============================================================

class TalentMasterCommandScript : public CommandScript
{
public:
    TalentMasterCommandScript() : CommandScript("TalentMasterCommandScript") {}

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable talentCommandTable =
        {
            { "reset",    HandleTalentResetCommand,    SEC_PLAYER, Console::No },
            { "resetpet", HandleTalentResetPetCommand, SEC_PLAYER, Console::No },
        };
        static ChatCommandTable commandTable =
        {
            { "talent", talentCommandTable },
        };
        return commandTable;
    }

    static bool HandleTalentResetCommand(ChatHandler* handler, char const* /*args*/)
    {
        if (!sConfigMgr->GetOption<bool>("TalentMaster.Command.Enable", true))
        {
            handler->SendSysMessage("Talent reset command is disabled.");
            return true;
        }

        Player* player = handler->GetPlayer();
        if (!player)
            return false;

        if (player->IsInCombat())
        {
            handler->SendSysMessage("You cannot reset talents while in combat.");
            return true;
        }

        if (player->CalculateTalentsPoints() == player->GetFreeTalentPoints())
        {
            handler->SendSysMessage("You have no talents to reset.");
            return true;
        }

        player->resetTalents(true);
        player->SendTalentsInfoData(false);
        handler->SendSysMessage("Your talents have been reset.");
        return true;
    }

    static bool HandleTalentResetPetCommand(ChatHandler* handler, char const* /*args*/)
    {
        if (!sConfigMgr->GetOption<bool>("TalentMaster.Command.Enable", true))
        {
            handler->SendSysMessage("Talent reset command is disabled.");
            return true;
        }

        Player* player = handler->GetPlayer();
        if (!player)
            return false;

        if (player->IsInCombat())
        {
            handler->SendSysMessage("You cannot reset pet talents while in combat.");
            return true;
        }

        Pet* pet = player->GetPet();
        if (!pet)
        {
            handler->SendSysMessage("You do not have an active pet.");
            return true;
        }

        if (pet->GetUsedTalentCount() == 0)
        {
            handler->SendSysMessage("Your pet has no talents to reset.");
            return true;
        }

        pet->resetTalents();
        player->SendTalentsInfoData(true);
        handler->SendSysMessage("Your pet's talents have been reset.");
        return true;
    }
};

// ============================================================
//  GossipScript / CreatureScript
// ============================================================

enum TalentMasterGossip
{
    GOSSIP_ACTION_RESET_PLAYER = 1,
    GOSSIP_ACTION_RESET_PET    = 2,
};

static constexpr char GOSSIP_MENU_RESET_PLAYER[]   = "I want to unlearn all my talents.";
static constexpr char GOSSIP_MENU_RESET_PET[]      = "I want to unlearn all my pet's talents.";
static constexpr char GOSSIP_MENU_CONFIRM_PLAYER[] = "Yes, reset all my talents for free.";
static constexpr char GOSSIP_MENU_CONFIRM_PET[]    = "Yes, reset all my pet's talents for free.";

class TalentMasterGossipScript : public CreatureScript
{
public:
    TalentMasterGossipScript() : CreatureScript("TalentMasterGossipScript") {}

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (!sConfigMgr->GetOption<bool>("TalentMaster.NPC.Enable", true))
            return false;

        ClearGossipMenuFor(player);

        AddGossipItemFor(player, GOSSIP_ICON_TALK, GOSSIP_MENU_RESET_PLAYER,
                         GOSSIP_SENDER_MAIN, GOSSIP_ACTION_RESET_PLAYER);

        if (player->getClass() == CLASS_HUNTER && player->GetPet())
        {
            AddGossipItemFor(player, GOSSIP_ICON_TALK, GOSSIP_MENU_RESET_PET,
                             GOSSIP_SENDER_MAIN, GOSSIP_ACTION_RESET_PET);
        }

        // Always send the menu — both hunters and non-hunters need it.
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        if (!sConfigMgr->GetOption<bool>("TalentMaster.NPC.Enable", true))
            return false;

        ClearGossipMenuFor(player);

        switch (action)
        {
            case GOSSIP_ACTION_RESET_PLAYER:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, GOSSIP_MENU_CONFIRM_PLAYER,
                                 GOSSIP_SENDER_MAIN, GOSSIP_ACTION_RESET_PLAYER + 100);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                break;

            case GOSSIP_ACTION_RESET_PET:
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, GOSSIP_MENU_CONFIRM_PET,
                                 GOSSIP_SENDER_MAIN, GOSSIP_ACTION_RESET_PET + 100);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                break;

            case GOSSIP_ACTION_RESET_PLAYER + 100:
                if (player->IsInCombat())
                {
                    CloseGossipMenuFor(player);
                    ChatHandler(player->GetSession()).SendSysMessage("You cannot reset talents while in combat.");
                    return true;
                }
                if (player->CalculateTalentsPoints() == player->GetFreeTalentPoints())
                {
                    CloseGossipMenuFor(player);
                    ChatHandler(player->GetSession()).SendSysMessage("You have no talents to reset.");
                    return true;
                }
                player->resetTalents(true);
                player->SendTalentsInfoData(false);
                CloseGossipMenuFor(player);
                ChatHandler(player->GetSession()).SendSysMessage("Your talents have been reset. Spend them wisely.");
                break;

            case GOSSIP_ACTION_RESET_PET + 100:
            {
                if (player->IsInCombat())
                {
                    CloseGossipMenuFor(player);
                    ChatHandler(player->GetSession()).SendSysMessage("You cannot reset pet talents while in combat.");
                    return true;
                }
                // Re-check: pet may have been dismissed between opening the
                // menu and clicking confirm.
                Pet* pet = player->GetPet();
                if (!pet)
                {
                    CloseGossipMenuFor(player);
                    ChatHandler(player->GetSession()).SendSysMessage("You no longer have an active pet.");
                    return true;
                }
                // pet->resetTalents() is a no-op if no talents are spent.
                if (pet->GetUsedTalentCount() == 0)
                {
                    CloseGossipMenuFor(player);
                    ChatHandler(player->GetSession()).SendSysMessage("Your pet has no talents to reset.");
                    return true;
                }
                pet->resetTalents();
                player->SendTalentsInfoData(true);
                CloseGossipMenuFor(player);
                ChatHandler(player->GetSession()).SendSysMessage("Your pet's talents have been reset.");
                break;
            }

            default:
                CloseGossipMenuFor(player);
                break;
        }

        return true;
    }
};

// ============================================================
//  WorldScript
// ============================================================

class TalentMasterWorld : public WorldScript
{
public:
    TalentMasterWorld() : WorldScript("TalentMasterWorld") {}

    void OnStartup() override
    {
        if (!sConfigMgr->GetOption<bool>("TalentMaster.NPC.Enable", true))
            return;

        uint32 entry = sConfigMgr->GetOption<uint32>("TalentMaster.NPC.Entry", 0);
        if (entry == 0)
        {
            LOG_WARN("server.loading", "mod-talentmaster: TalentMaster.NPC.Enable is on but TalentMaster.NPC.Entry is 0.");
            return;
        }

        if (!sObjectMgr->GetCreatureTemplate(entry))
        {
            LOG_ERROR("server.loading", "mod-talentmaster: TalentMaster.NPC.Entry {} not found in creature_template.", entry);
            return;
        }

        LOG_INFO("server.loading", "mod-talentmaster: NPC gossip active on creature entry {}.", entry);
    }
};

// Standard Automatic Script Loader
void Addmod_talentmasterScripts()
{
    new TalentMasterWorld();
    new TalentMasterCommandScript();
    new TalentMasterGossipScript();
}
