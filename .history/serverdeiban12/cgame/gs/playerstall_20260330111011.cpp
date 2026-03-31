#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arandomgen.h>

#include <vector>

#include <common/protocol.h>
#include "world.h"
#include "player_imp.h"
#include "usermsg.h"
#include "clstab.h"
#include "actsession.h"
#include "userlogin.h"
#include "playertemplate.h"
#include <common/protocol_imp.h>
#include "playerstall.h"
#include "serviceprovider.h"
#include "config.h"
#include "stall_website_sync.h"
#include "worldmanager.h"

void gplayer_imp::stall_website_sync_emit(const char *action)
{
	if (!action)
		return;
	gplayer *pPlayer = (gplayer *)_parent;
	if (!pPlayer)
		return;
	int world_tag = world_manager::GetWorldTag();
	std::vector<stall_sync_goods_entry> goods;
	if (_stall_obj)
	{
		for (size_t i = 0; i < _stall_obj->_goods_list.size(); i++)
		{
			const player_stall::entry_t &e = _stall_obj->_goods_list[i];
			if (!e.count)
				continue;
			stall_sync_goods_entry g;
			g.item_id = e.type;
			g.count = (int)e.count;
			g.price = (unsigned int)e.price;
			g.side = (e.index == 0xFFFF) ? 1 : 0;
			goods.push_back(g);
		}
	}
	unsigned int uname_len = 0;
	const void *uname = GetPlayerName(uname_len);
	stall_website_sync_cat_notify(
		pPlayer->ID.id,
		_stall_trade_id,
		world_tag,
		pPlayer->pos.x,
		pPlayer->pos.y,
		pPlayer->pos.z,
		_stall_obj ? _stall_obj->GetName() : NULL,
		_stall_obj ? _stall_obj->GetNameLen() : 0,
		uname,
		uname_len,
		goods.empty() ? NULL : &goods[0],
		goods.size(),
		action);
}

bool gplayer_imp::PlayerOpenPersonalMarket(size_t count, const char name[PLAYER_MARKET_MAX_NAME_LEN], int *entry_list)
{
	if (_parent->IsZombie() || _player_state != PLAYER_STATE_NORMAL)
		return false;
	if (!CanSitDown())
		return false;
	if (_cur_session || _session_list.size())
		return false;
	if (count > PLAYER_MARKET_MAX_SELL_SLOT + PLAYER_MARKET_MAX_BUY_SLOT || count == 0)
		return false;
	if (_cheat_punish)
		return false;

	//?????????????
	int name_len;
	for (name_len = 0; name_len < PLAYER_MARKET_MAX_NAME_LEN; name_len += 2)
	{
		if (name[name_len] == 0 && name[name_len + 1] == 0)
		{
			name_len += 2;
			break;
		}
	}
	if (name_len > _stall_info.max_name_length)
		return false;

	packet_wrapper h1(1024);
	using namespace S2C;
	CMD::Make<CMD::self_open_market>::From(h1, count);
	//???????
	//??????????????

	abase::vector<char> flag_list;
	flag_list.insert(flag_list.begin(), _inventory.Size(), 0);

	int order_count = 0;
	size_t sell_money = 0;
	size_t buy_money = 0;
	C2S::CMD::open_personal_market::entry_t *ent = (C2S::CMD::open_personal_market::entry_t *)entry_list;
	for (size_t i = 0; i < count; i++)
	{
		if (ent[i].price == 0 || ent[i].price > 2000000000u || ent[i].count == 0 ||
			(ent[i].index != 0xFFFF && !_inventory.IsItemExist(ent[i].index, ent[i].type, ent[i].count)))
		{
			//?????????????
			_runner->error_message(S2C::ERR_INVALID_ITEM);
			return false;
		}
		if (ent[i].index == 0xFFFF)
		{
			item_data *pData = (item_data *)world_manager::GetDataMan().get_item_for_sell(ent[i].type);
			if (pData == NULL || ent[i].count > pData->pile_limit)
			{
				_runner->error_message(S2C::ERR_INVALID_ITEM);
				return false;
			}
			//??????????????????????????
			order_count++;
			//????????????
			size_t p = ent[i].price * ent[i].count;
			if (p / ent[i].count != ent[i].price)
				return false;
			size_t tmp = buy_money + p;
			if (tmp < buy_money)
				return false;
			buy_money = tmp;
		}
		else
		{
			if (_inventory[ent[i].index].proc_type & item::ITEM_PROC_TYPE_NOTRADE)
			{
				//??????????????????
				_runner->error_message(S2C::ERR_INVALID_ITEM);
				return false;
			}

			if (flag_list[ent[i].index]++)
			{
				//????????????????????
				_runner->error_message(S2C::ERR_INVALID_ITEM);
				return false;
			}
			//?????????????
			size_t p = ent[i].price * ent[i].count;
			if (p / ent[i].count != ent[i].price)
				return false;
			size_t tmp = sell_money + p;
			if (tmp < sell_money)
				return false;
			sell_money = tmp;
		}
	}

	//??????????????
	if (order_count > _stall_info.max_buy_slot || (int)count - order_count > _stall_info.max_sell_slot)
		return false;

	//?????????????????
	if (_stall_info.stallcard_id != -1 && (buy_money > _player_money || sell_money > 4000000000u))
		return false;
	if (_stall_info.stallcard_id == -1 && (buy_money > _player_money || sell_money > MONEY_CAPACITY_BASE - _player_money))
		return false;

	//???????????????
	//???????????
	int sys_time = g_timer.get_systime();
	if (sys_time == _stall_trade_timer)
		return false;
	_stall_trade_timer = sys_time;

	//?????????????????????????????????????
	ASSERT(!_stall_obj);

	//?????????????????
	_petman.RecallPet(this);

	GLog::log(GLOG_INFO, "???%d??????", _parent->ID.id);

	_stall_obj = new player_stall(_inventory);
	_stall_obj->SetMarketName(name);
	_stall_obj->SetSlot(count - (size_t)order_count, (size_t)order_count);
	for (size_t i = 0; i < count; i++)
	{
		CMD::Make<CMD::self_open_market>::AddGoods(h1, ent[i].type, ent[i].index, ent[i].count, ent[i].price);
		if (ent[i].index == 0xFFFF)
			_stall_obj->AddOrderGoods(ent[i].index, ent[i].type, ent[i].count, ent[i].price);
		else
			_stall_obj->AddTradeGoods(ent[i].index, ent[i].type, ent[i].count, ent[i].price);
	}

	_player_state = PLAYER_STATE_MARKET;
	_stall_trade_id++;
	if ((_stall_trade_id & 0xFF) == 0)
		_stall_trade_id++;
	//??????????
	gplayer *pPlayer = (gplayer *)_parent;
	pPlayer->market_id = _stall_trade_id & 0xFF;
	pPlayer->object_state |= gactive_object::STATE_MARKET;

	//??????????????
	send_ls_msg(pPlayer, h1);

	//?????????
	_runner->begin_personal_market(_stall_trade_id, _stall_obj->GetName(), _stall_obj->GetNameLen());
	stall_website_sync_emit("open");
	return true;
}

bool gplayer_imp::PlayerTestPersonalMarket()
{
	if (_parent->IsZombie() || _player_state != PLAYER_STATE_NORMAL)
		return false;
	if (_cur_session || _session_list.size())
		return false;
	if (!CanSitDown())
		return false;
	_runner->personal_market_available();
	return true;
}

bool gplayer_imp::CancelPersonalMarket()
{
	//?????????????????????????
	//???????????????????????????????

	GLog::log(GLOG_INFO, "???%d?????", _parent->ID.id);

	if (_stall_obj)
		stall_website_sync_emit("close");
	//???????????
	delete _stall_obj;
	_stall_obj = NULL;

	//????????????
	_player_state = PLAYER_STATE_NORMAL;
	gplayer *pPlayer = (gplayer *)_parent;
	pPlayer->object_state &= ~(gactive_object::STATE_MARKET);

	//??
	_runner->cancel_personal_market();
	return true;
}

int gplayer_imp::DoPlayerMarketTrade(const XID &trader, const XID &buyer, gplayer *pTrader, gplayer *pBuyer, const void *order, size_t length)
{
	//???????????????????
	if (!pTrader->IsActived() || pTrader->ID != trader || pTrader->imp == NULL || pTrader->IsZombie())
	{
		return -1;
	}

	if (!pBuyer->IsActived() || pBuyer->ID != buyer || pBuyer->imp == NULL || pBuyer->IsZombie())
	{
		return 1;
	}

	//?????,???????????????
	player_stall::trade_request &req = *(player_stall::trade_request *)order;
	if (length < sizeof(req) || length != sizeof(req) + req.count * sizeof(player_stall::trade_request::entry_t))
	{
		return -2;
	}

	// req.yinpiao???????????????????need_money?????????????,money_to_yinpiao????????????????????????????
	//??????req.yinpiao+money_to_yinpiao???????need_money-money_to_yinpiao*10000000???
	size_t need_money = 0;
	size_t money_to_yinpiao = 0;
	//??????????? ??????????
	if (!((gplayer_imp *)(pTrader->imp))->CheckMarketTradeRequest(req, need_money, money_to_yinpiao, (gplayer_imp *)(pBuyer->imp)))
	{
		return -3;
	}

	//??????????????????????,???????????
	if (!((gplayer_imp *)(pBuyer->imp))->CheckMarketTradeRequire(req, need_money, (gplayer_imp *)(pTrader->imp)))
	{
		return -4;
	}

	//???????????,???????????
	((gplayer_imp *)(pTrader->imp))->DoPlayerMarketTrade(req, (gplayer_imp *)(pBuyer->imp), need_money, money_to_yinpiao);
	return 0;
}

int gplayer_imp::DoPlayerMarketPurchase(const XID &trader, const XID &buyer, gplayer *pTrader, gplayer *pBuyer, const void *order, size_t length)
{
	//???????????????????
	if (!pTrader->IsActived() || pTrader->ID != trader || pTrader->imp == NULL || pTrader->IsZombie())
	{
		return -1;
	}

	if (!pBuyer->IsActived() || pBuyer->ID != buyer || pBuyer->imp == NULL || pBuyer->IsZombie())
	{
		return 1;
	}

	//?????,???????????????
	player_stall::trade_request &req = *(player_stall::trade_request *)order;
	if (length < sizeof(req) || length != sizeof(req) + req.count * sizeof(player_stall::trade_request::entry_t))
	{
		return -2;
	}

	// req.yinpiao???????????????????total_price?????????????,money_to_yinpiao????????????????????????????
	//???????req.yinpiao+money_to_yinpiao???????total_price-money_to_yinpiao*10000000???
	req.yinpiao = 0;
	size_t total_price = 0;
	size_t money_to_yinpiao = 0;
	//???????????????????????????????????? ????????????????????
	if (!((gplayer_imp *)(pTrader->imp))->CheckMarketPurchaseRequest(req, total_price, (gplayer_imp *)(pBuyer->imp)))
	{
		return -3;
	}

	//?????????????????????????????
	if (!((gplayer_imp *)(pBuyer->imp))->CheckMarketPurchaseRequire(req, total_price, money_to_yinpiao, (gplayer_imp *)(pTrader->imp)))
	{
		return -4;
	}

	//???????????,???????????
	((gplayer_imp *)(pTrader->imp))->DoPlayerMarketPurchase(req, (gplayer_imp *)(pBuyer->imp), total_price, money_to_yinpiao);
	return 0;
}
int gplayer_imp::MarketHandler(world *pPlane, const MSG &msg)
{
	switch (msg.message)
	{
	case GM_MSG_SWITCH_GET:
		//??????
		break;

	case GM_MSG_QUERY_PERSONAL_MARKET_NAME:
		_runner->send_market_name(msg.source, *(int *)(msg.content), msg.param, _stall_obj->GetName(), _stall_obj->GetNameLen());
		break;

	case GM_MSG_PICKUP_MONEY:
	case GM_MSG_RECEIVE_MONEY:
	case GM_MSG_PICKUP_ITEM:
	case GM_MSG_HATE_YOU:
	case GM_MSG_HEARTBEAT:
	case GM_MSG_QUERY_OBJ_INFO00:
	case GM_MSG_ERROR_MESSAGE:
	case GM_MSG_GROUP_EXPERIENCE:
	case GM_MSG_EXPERIENCE:
	case GM_MSG_TEAM_EXPERIENCE:
	case GM_MSG_GET_MEMBER_POS:
	case GM_MSG_TEAM_INVITE:
	case GM_MSG_TEAM_AGREE_INVITE:
	case GM_MSG_TEAM_REJECT_INVITE:
	case GM_MSG_JOIN_TEAM:
	case GM_MSG_LEADER_UPDATE_MEMBER:
	case GM_MSG_JOIN_TEAM_FAILED:
	case GM_MSG_MEMBER_NOTIFY_DATA:
	case GM_MSG_NEW_MEMBER:
	case GM_MSG_LEAVE_PARTY_REQUEST:
	case GM_MSG_LEADER_CANCEL_PARTY:
	case GM_MSG_MEMBER_NOT_IN_TEAM:
	case GM_MSG_LEADER_KICK_MEMBER:
	case GM_MSG_MEMBER_LEAVE:
	case GM_MSG_QUERY_PLAYER_EQUIPMENT:
	case GM_MSG_PICKUP_TEAM_MONEY:
	case GM_MSG_NOTIFY_SELECT_TARGET:
	case GM_MSG_QUERY_SELECT_TARGET:

	case GM_MSG_NPC_BE_KILLED:
	case GM_MSG_PLAYER_TASK_TRANSFER:
	case GM_MSG_PLAYER_BECOME_PARIAH:
	case GM_MSG_PLAYER_BECOME_INVADER:
	case GM_MSG_SUBSCIBE_TARGET:
	case GM_MSG_UNSUBSCIBE_TARGET:
	case GM_MSG_SUBSCIBE_CONFIRM:
	case GM_MSG_SUBSCIBE_SUBTARGET:
	case GM_MSG_UNSUBSCIBE_SUBTARGET:
	case GM_MSG_SUBSCIBE_SUBTARGET_CONFIRM:
	case GM_MSG_SUBSCIBE_ENEMY:
	case GM_MSG_UNSUBSCIBE_ENEMY:
	case GM_MSG_NOTIFY_SELECT_SUBTARGET:
	case GM_MSG_HP_STEAL:
	case GM_MSG_TEAM_APPLY_PARTY:
	case GM_MSG_TEAM_APPLY_REPLY:
	case GM_MSG_QUERY_INFO_1:
	case GM_MSG_TEAM_CHANGE_TO_LEADER:
	case GM_MSG_TEAM_LEADER_CHANGED:
	case GM_MSG_DBSAVE_ERROR:
	case GM_MSG_PLAYER_DUEL_REQUEST:
	case GM_MSG_PLAYER_DUEL_REPLY:
	case GM_MSG_PLAYER_DUEL_PREPARE:
	case GM_MSG_PLAYER_DUEL_START:
	case GM_MSG_PLAYER_DUEL_CANCEL:
	case GM_MSG_PLAYER_DUEL_STOP:
	case GM_MSG_PLAYER_BIND_REQUEST:
	case GM_MSG_PLAYER_BIND_INVITE:
	case GM_MSG_QUERY_EQUIP_DETAIL:

	case GM_MSG_PLAYER_RECALL_PET:
	case GM_MSG_MOB_BE_TRAINED:
	case GM_MSG_PET_SET_COOLDOWN:
	case GM_MSG_PET_ANTI_CHEAT:
	case GM_MSG_PET_NOTIFY_DEATH:
	case GM_MSG_PET_NOTIFY_HP:
	case GM_MSG_PET_RELOCATE_POS:
	case GM_MSG_QUERY_PROPERTY:
	case GM_MSG_QUERY_PROPERTY_REPLY:
	case GM_MSG_PLANT_PET_NOTIFY_DEATH:
	case GM_MSG_PLANT_PET_NOTIFY_HP:
	case GM_MSG_PLANT_PET_NOTIFY_DISAPPEAR:
	case GM_MSG_CONGREGATE_REQUEST:
	case GM_MSG_REJECT_CONGREGATE:
	case GM_MSG_NPC_BE_KILLED_BY_OWNER:
	case GM_MSG_EXTERN_HEAL:
	case GM_MSG_QUERY_INVENTORY_DETAIL:
	case GM_MSG_PLAYER_KILLED_BY_PLAYER:

		//??????GM???
	case GM_MSG_GM_RECALL:
	case GM_MSG_GM_CHANGE_EXP:
	case GM_MSG_GM_OFFLINE:
	case GM_MSG_GM_MQUERY_MOVE_POS:
	case GM_MSG_GM_DEBUG_COMMAND:
	case GM_MSG_GM_QUERY_SPEC_ITEM:
	case GM_MSG_LOOKUP_ENEMY:
	case GM_MSG_LOOKUP_ENEMY_REPLY:

		//?????????????????????????????
		return DispatchNormalMessage(pPlane, msg);

	case GM_MSG_ENCHANT:
		if (((enchant_msg *)msg.content)->helpful)
			return DispatchNormalMessage(pPlane, msg);
	case GM_MSG_ATTACK:
	case GM_MSG_HURT:
	case GM_MSG_DUEL_HURT:
	case GM_MSG_TRANSFER_FILTER_DATA:
	case GM_MSG_TRANSFER_FILTER_GET:
		//????????????????????
		//????????????????? ??????????????????
		return DispatchNormalMessage(pPlane, msg);

	case GM_MSG_SERVICE_HELLO:
	{
		if (msg.pos.squared_distance(_parent->pos) < 36.f) //????????
		{
			SendTo<0>(GM_MSG_SERVICE_GREETING, msg.source, 0);
		}
	}
		return 0;

	case GM_MSG_SERVICE_REQUEST:
		if (msg.pos.squared_distance(_parent->pos) > 36.f) // 6??????
		{
			SendTo<0>(GM_MSG_ERROR_MESSAGE, msg.source, S2C::ERR_SERVICE_UNAVILABLE);
			return 0;
		}
		//????????????(??????)
		if (msg.param == service_ns::SERVICE_ID_PLAYER_MARKET || msg.param == service_ns::SERVICE_ID_PLAYER_MARKET2)
		{
			//????ID???,??????????????????
			int index1 = 0;
			gplayer *pParent = GetParent();
			XID self = pParent->ID;
			gplayer *pPlayer1 = world_manager::GetInstance()->FindPlayer(msg.source.id, index1);
			if (!pPlayer1 || pPlayer1 == pParent || _plane != world_manager::GetInstance()->GetWorldByIndex(index1))
			{
				SendTo<0>(GM_MSG_ERROR_MESSAGE, msg.source, S2C::ERR_SERVICE_UNAVILABLE);
				return 0;
			}

			//?????????????????
			if (pPlayer1 < pParent)
			{
				if (mutex_spinset(&pPlayer1->spinlock) != 0)
				{
					//????????
					//???????????????
					mutex_spinunlock(&pParent->spinlock);
					mutex_spinlock(&pPlayer1->spinlock);
					mutex_spinlock(&pParent->spinlock);
				}
			}
			else
			{
				//???????????
				mutex_spinlock(&pPlayer1->spinlock);
			}

			world *pPlane = _plane;
			//?????????????????
			if (msg.param == service_ns::SERVICE_ID_PLAYER_MARKET)
			{
				//????????????
				if (DoPlayerMarketTrade(self, msg.source, pParent, pPlayer1, msg.content, msg.content_length) < 0)
				{
					//??????????? ??????????SendTo???????????????????????
					MSG msg2;
					BuildMessage(msg2, GM_MSG_ERROR_MESSAGE, msg.source, self, A3DVECTOR(0, 0, 0), S2C::ERR_SERVICE_UNAVILABLE);
					pPlane->PostLazyMessage(msg2);
				}
			}
			else
			{
				//???????????????
				if (DoPlayerMarketPurchase(self, msg.source, pParent, pPlayer1, msg.content, msg.content_length) < 0)
				{
					//??????????? ??????????SendTo???????????????????????
					MSG msg2;
					BuildMessage(msg2, GM_MSG_ERROR_MESSAGE, msg.source, self, A3DVECTOR(0, 0, 0), S2C::ERR_SERVICE_UNAVILABLE);
					pPlane->PostLazyMessage(msg2);
				}
			}

			//???????
			mutex_spinunlock(&pPlayer1->spinlock);
		}
		else
		{
			//???????
			SendTo<0>(GM_MSG_ERROR_MESSAGE, msg.source, S2C::ERR_SERVICE_UNAVILABLE);
			return 0;
		}
		return 0;

	case GM_MSG_SERVICE_QUIERY_CONTENT:
		if (msg.pos.squared_distance(_parent->pos) > 36.f) // 6??????
		{
			SendTo<0>(GM_MSG_ERROR_MESSAGE, msg.source, S2C::ERR_SERVICE_UNAVILABLE);
			return 0;
		}
		if (msg.content_length == sizeof(int) * 2)
		{
			int cs_index = *(int *)msg.content;
			int sid = *((int *)msg.content + 1);
			//???????????????????
			ASSERT(_stall_obj);
			if (!_stall_obj)
				return 0;
			packet_wrapper h1(2048);
			using namespace S2C;
			size_t count = _stall_obj->_goods_list.size();
			CMD::Make<CMD::player_market_info>::From(h1, _parent, _stall_trade_id, count);
			for (size_t i = 0; i < count; i++)
			{
				const player_stall::entry_t &ent = _stall_obj->_goods_list[i];
				if (ent.count)
				{
					if (ent.index == 0xFFFF)
					{
						//???????
						CMD::Make<INFO::market_goods>::BuyItem(h1, ent.type, ent.count, ent.price);
						continue;
					}

					size_t index = ent.index;
					item_data data;
					unsigned short crc;
					if (_inventory.GetItemData(index, data, crc) > 0 && ent.type == data.type && crc == (unsigned short)ent.crc && data.expire_date == ent.expire_date)
					{
						CMD::Make<INFO::market_goods>::SellItem(h1, ent.type, ent.count, ent.price, data);
						continue;
					}
				}
				CMD::Make<INFO::market_goods>::From(h1); //????????????
			}
			send_ls_msg(cs_index, msg.source.id, sid, h1.data(), h1.size());
		}
		else
		{
			ASSERT(false);
		}
		return 0;
	}
	return 0;
}

bool gplayer_imp::CheckMarketTradeRequest(player_stall::trade_request &req, size_t &need_money, size_t &money_to_yinpiao, gplayer_imp *pBuyerImp)
{
	//?????????????????
	if (_player_state != PLAYER_STATE_MARKET)
		return false;
	if (!_stall_obj)
	{
		ASSERT(false);
		return false;
	}
	if (_stall_trade_id != req.trade_id)
		return false;
	size_t m = 0;
	abase::vector<int, abase::fast_alloc<>> list;
	list.insert(list.begin(), _stall_obj->_goods_list.size(), 0);
	for (size_t i = 0; i < req.count; i++)
	{
		size_t index = req.list[i].index;
		if (index >= _stall_obj->_goods_list.size())
			return false;
		const player_stall::entry_t &ent = _stall_obj->_goods_list[index];
		size_t item_count = req.list[i].count;
		if (item_count == 0)
			return false;
		if (ent.index == 0xFFFF)
			return false; //??????????
		if (ent.type != req.list[i].type)
			return false;
		if (ent.count < item_count || ent.count < list[index] + item_count)
		{
			//????????????
			return false;
		}
		list[index] += item_count;

		if (!_inventory.IsItemExist(ent.index, ent.type, ent.count))
			return false;
		//???????????????
		if (_inventory[ent.index].proc_type & item::ITEM_PROC_TYPE_NOTRADE)
			return false;
		if (ent.crc != _inventory[ent.index].GetCRC())
			return false;
		if (ent.expire_date != _inventory[ent.index].expire_date)
			return false;

		//??????????
		size_t p = ent.price * item_count;
		if (p / item_count != ent.price)
			return false;
		size_t tmp = m + p;
		if (tmp < m)
			return false;
		m = tmp;
	}

	//?????????????????
	if (req.yinpiao)
	{
		if (_stall_info.stallcard_id == -1)
			return false; //?????????????????????????
		if (req.yinpiao > 400)
			return false; //??????
		if (req.yinpiao * WANMEI_YINPIAO_PRICE > m)
			return false;
		m -= req.yinpiao * WANMEI_YINPIAO_PRICE;
	}
	//????????????????????????????
	if (_stall_info.stallcard_id != -1 && m >= WANMEI_YINPIAO_PRICE) //????????????????????????????
		money_to_yinpiao = m / WANMEI_YINPIAO_PRICE;
	else
		money_to_yinpiao = 0;
	//??????????????
	if (req.yinpiao + money_to_yinpiao)
	{
		if (world_manager::GetDataMan().get_item_sell_price(WANMEI_YINPIAO_ID) != WANMEI_YINPIAO_PRICE)
			return false;
		if ((int)(req.yinpiao + money_to_yinpiao) > world_manager::GetDataMan().get_item_pile_limit(WANMEI_YINPIAO_ID))
			return false;
		if (!_inventory.HasSlot(WANMEI_YINPIAO_ID, (int)(req.yinpiao + money_to_yinpiao)))
		{
			pBuyerImp->_runner->error_message(S2C::ERR_TRADER_MONEY_REACH_UPPER_LIMIT);
			return false;
		}
	}

	size_t m_total = m - money_to_yinpiao * WANMEI_YINPIAO_PRICE + GetMoney();
	if (m_total < GetMoney() || m_total > _money_capacity)
	{
		//??????????
		// CancelPersonalMarket();	//?????????????????????????????????
		pBuyerImp->_runner->error_message(S2C::ERR_TRADER_MONEY_REACH_UPPER_LIMIT);
		return false;
	}
	//?????????????????
	need_money = m;
	return true;
}

bool gplayer_imp::CheckMarketPurchaseRequest(player_stall::trade_request &req, size_t &total_price, gplayer_imp *pBuyerImp)
{
	//?????????????????
	if (_player_state != PLAYER_STATE_MARKET)
		return false;
	if (!_stall_obj)
	{
		ASSERT(false);
		return false;
	}
	if (_stall_trade_id != req.trade_id)
		return false;
	size_t m = 0;
	abase::vector<int, abase::fast_alloc<>> list;
	list.insert(list.begin(), _stall_obj->_goods_list.size(), 0);
	for (size_t i = 0; i < req.count; i++)
	{
		size_t index = req.list[i].index;
		if (index >= _stall_obj->_goods_list.size())
			return false;
		size_t item_count = req.list[i].count;
		const player_stall::entry_t &ent = _stall_obj->_goods_list[index];
		if (item_count == 0)
			return false; //??????????
		if (ent.index != 0xFFFF)
			return false; //??????????
		if (ent.type != req.list[i].type)
			return false; //???????????????
		if (ent.count < item_count || ent.count < item_count + list[index])
		{
			//????????????????
			return false;
		}
		list[index] += item_count;

		//??????????
		size_t p = ent.price * item_count;
		if (p / item_count != ent.price)
			return false; //?????????????
		size_t tmp = m + p;
		if (tmp < m)
			return false; //?????
		m = tmp;
	}

	if (_stall_info.stallcard_id != -1 && m >= WANMEI_YINPIAO_PRICE) //????????????????????
	{
		req.yinpiao = m / WANMEI_YINPIAO_PRICE;
		int max = GetItemCount(WANMEI_YINPIAO_ID);
		if ((int)req.yinpiao > max)
			req.yinpiao = max;
		m -= req.yinpiao * WANMEI_YINPIAO_PRICE;
	}
	else
		req.yinpiao = 0;
	if (GetMoney() < m)
	{
		//???????????????
		pBuyerImp->_runner->error_message(S2C::ERR_TRADER_MONEY_ISNOT_ENOUGH);
		return false;
	}

	//????????????????????
	if (_inventory.GetEmptySlotCount() < req.count)
		return false;

	//?????????????????
	total_price = m;
	return true;
}

bool gplayer_imp::CheckMarketPurchaseRequire(player_stall::trade_request &req, size_t total_price, size_t &money_to_yinpiao, gplayer_imp *pTraderImp)
{
	//?????????????????
	if (_player_state != PLAYER_STATE_NORMAL)
		return false;
	if (OI_TestSafeLock())
		return false;

	//???????????????????????????
	if (pTraderImp->_stall_info.stallcard_id != -1 && total_price >= WANMEI_YINPIAO_PRICE) //????????????????????????????
		money_to_yinpiao = total_price / WANMEI_YINPIAO_PRICE;
	else
		money_to_yinpiao = 0;
	//??????????????
	if (req.yinpiao + money_to_yinpiao)
	{
		if (world_manager::GetDataMan().get_item_sell_price(WANMEI_YINPIAO_ID) != WANMEI_YINPIAO_PRICE)
			return false;
		if ((int)(req.yinpiao + money_to_yinpiao) > world_manager::GetDataMan().get_item_pile_limit(WANMEI_YINPIAO_ID))
			return false;
		if (!_inventory.HasSlot(WANMEI_YINPIAO_ID, (int)(req.yinpiao + money_to_yinpiao)))
		{
			_runner->error_message(S2C::ERR_BUYER_MONEY_REACH_UPPER_LIMIT);
			return false;
		}
	}
	size_t m_total = total_price - money_to_yinpiao * WANMEI_YINPIAO_PRICE + GetMoney();
	if (m_total < GetMoney() || m_total > _money_capacity)
	{
		//??????????????
		_runner->error_message(S2C::ERR_BUYER_MONEY_REACH_UPPER_LIMIT);
		return false;
	}

	//?????????????
	abase::vector<int, abase::fast_alloc<>> list;
	list.insert(list.begin(), _inventory.Size(), 0);
	for (size_t i = 0; i < req.count; i++)
	{
		int type = req.list[i].type;
		size_t count = req.list[i].count;
		size_t inv_index = req.list[i].inv_index;
		if (!_inventory.IsItemExist(inv_index, type, count))
			return false;
		//??????????????????
		if (_inventory[inv_index].proc_type & item::ITEM_PROC_TYPE_NOTRADE)
			return false;
		if (list[inv_index])
			return false;
		list[inv_index] = 1; //???????????????
	}

	//????????????????????????
	return true;
}

bool gplayer_imp::CheckMarketTradeRequire(player_stall::trade_request &req, size_t need_money, gplayer_imp *pTraderImp)
{
	//?????????????????
	if (_player_state != PLAYER_STATE_NORMAL)
		return false;
	if (OI_TestSafeLock())
		return false;
	if (GetMoney() < need_money)
		return false; //?????
	if (req.yinpiao && !CheckItemExist(WANMEI_YINPIAO_ID, req.yinpiao))
		return false; //???????
	if (pTraderImp->_stall_info.stallcard_id != -1 && need_money >= WANMEI_YINPIAO_PRICE && GetItemCount(WANMEI_YINPIAO_ID) > (int)req.yinpiao)
		return false; //??????????????????????????????????

	//????????????????
	if (_inventory.GetEmptySlotCount() < req.count)
		return false;
	return true;
}

//?????????????????
void gplayer_imp::DoPlayerMarketTrade(player_stall::trade_request &req, gplayer_imp *pImp, size_t need_money, size_t money_to_yinpiao)
{
	item_list &inv = pImp->_inventory;

	packet_wrapper h1(128);
	using namespace S2C;
	CMD::Make<CMD::player_purchase_item>::FirstStep(h1, need_money, req.yinpiao, req.count);

	//??????
	//???????????????
	for (size_t i = 0; i < req.count; i++)
	{
		size_t item_index = req.list[i].index;
		ASSERT(item_index < _stall_obj->_goods_list.size());
		player_stall::entry_t &ent = _stall_obj->_goods_list[item_index];
		size_t item_count = req.list[i].count;
		ASSERT(item_count);
		ASSERT(ent.type == req.list[i].type && ent.count >= item_count);
		ASSERT(_inventory.IsItemExist(ent.index, ent.type, ent.count));
		ASSERT(ent.crc == _inventory[ent.index].GetCRC());

		item it;
		if (_inventory[ent.index].count == item_count)
		{
			_inventory.Exchange(ent.index, it);
		}
		else
		{
			//?????????
			it = _inventory[ent.index];
			if (it.body)
				it.body = it.body->Clone();
			it.count = item_count;
			_inventory.DecAmount(ent.index, item_count);
		}
		//??????????????
		ent.count -= item_count;
		if (ent.count == 0)
			_stall_obj->DecSellSlot();
		_runner->trade_away_item(pImp->GetParent()->ID.id, ent.index, ent.type, item_count);

		int expire_date = it.expire_date;
		int rst = inv.Push(it);
		if (rst < 0)
		{
			ASSERT(false);
			//???????, ???????????,?????? contine
			it.Release();
			continue;
		}
		GLog::log(GLOG_INFO, "???%d???????%d %d??%d??????%d", _parent->ID.id, pImp->_parent->ID.id, item_count, ent.type, ent.price);

		//??????????????????
		CMD::Make<CMD::player_purchase_item>::SecondStep(h1, ent.type, expire_date, item_count, rst);
	}

	//?????????????????
	//????????????????????????
	ASSERT(need_money - money_to_yinpiao * WANMEI_YINPIAO_PRICE + _player_money <= _money_capacity && need_money - money_to_yinpiao * WANMEI_YINPIAO_PRICE + _player_money >= _player_money);
	pImp->SpendMoney(need_money);
	GainMoney(need_money - money_to_yinpiao * WANMEI_YINPIAO_PRICE);

	_runner->get_player_money(GetMoney(), _money_capacity, _color_name);
	send_ls_msg(pImp->GetParent(), h1);
	//?????????
	if (req.yinpiao)
	{
		pImp->RemoveItems(WANMEI_YINPIAO_ID, req.yinpiao, S2C::DROP_TYPE_TRADEAWAY, false);
	}
	//????????
	if (req.yinpiao + money_to_yinpiao)
	{
		element_data::item_tag_t tag = {element_data::IMT_NULL, 0};
		item_data *data = world_manager::GetDataMan().generate_item_from_player(WANMEI_YINPIAO_ID, &tag, sizeof(tag));
		if (data == NULL)
		{
			ASSERT(false);
			return;
		}
		data->count = req.yinpiao + money_to_yinpiao;
		int rst = _inventory.Push(*data);
		if (rst < 0 || data->count)
		{
			ASSERT(false);
			FreeItem(data);
			return;
		}
		_runner->obtain_item(WANMEI_YINPIAO_ID, 0, req.yinpiao + money_to_yinpiao, _inventory[rst].count, IL_INVENTORY, rst);
		GLog::log(GLOG_INFO, "???%d(???)???????%d(???)???,????%d?????,?????????????????%d???", pImp->_parent->ID.id, _parent->ID.id, req.yinpiao, money_to_yinpiao);
		FreeItem(data);
	}

	//???????????
	_runner->market_trade_success(pImp->GetParent()->ID.id);
	pImp->_runner->market_trade_success(GetParent()->ID.id);

	//??????????????????
	pImp->ReduceSaveTimer(100);
	ReduceSaveTimer(100);
	stall_website_sync_emit("update");
}

//????????
void gplayer_imp::DoPlayerMarketPurchase(player_stall::trade_request &req, gplayer_imp *pImp, size_t total_price, size_t money_to_yinpiao)
{
	item_list &inv = pImp->_inventory;

	packet_wrapper h1(128);
	using namespace S2C;
	CMD::Make<CMD::player_purchase_item>::FirstStep(h1, total_price, req.yinpiao, req.count, false);

	//??????
	//???????????????
	for (size_t i = 0; i < req.count; i++)
	{

		size_t item_index = req.list[i].inv_index;
		size_t item_count = req.list[i].count;
		size_t stall_index = req.list[i].index;
		ASSERT(item_count);
		ASSERT(inv.IsItemExist(item_index, req.list[i].type, item_count));
		player_stall::entry_t &ent = _stall_obj->_goods_list[stall_index];
		ASSERT(ent.type == req.list[i].type && ent.count >= item_count);

		item it;
		if (inv[item_index].count == item_count)
		{
			inv.Remove(item_index, it);
		}
		else
		{
			//?????????
			it = inv[item_index];
			if (it.body)
				it.body = it.body->Clone();
			it.count = item_count;
			inv.DecAmount(item_index, item_count);
		}

		//??????????????
		ent.count -= item_count;
		if (ent.count == 0)
			_stall_obj->DecBuySlot();
		pImp->_runner->player_drop_item(IL_INVENTORY, item_index, ent.type, item_count, S2C::DROP_TYPE_TRADEAWAY);

		int expire_date = it.expire_date;
		int rst = _inventory.Push(it);
		if (rst < 0)
		{
			ASSERT(false);
			//???????, ???????????,?????? contine
			it.Release();
			continue;
		}
		GLog::log(GLOG_INFO, "???%d??????%d %d??%d??????%d", _parent->ID.id, pImp->_parent->ID.id, item_count, ent.type, ent.price);

		//??????????????????
		CMD::Make<CMD::player_purchase_item>::SecondStep(h1, ent.type, expire_date, item_count, rst, stall_index & 0xFF);
	}

	//?????????????????
	ASSERT(total_price - money_to_yinpiao * WANMEI_YINPIAO_PRICE + pImp->_player_money <= pImp->_money_capacity && total_price - money_to_yinpiao * WANMEI_YINPIAO_PRICE + pImp->_player_money >= pImp->_player_money);
	SpendMoney(total_price);
	pImp->GainMoney(total_price - money_to_yinpiao * WANMEI_YINPIAO_PRICE);
	//?????????
	if (req.yinpiao)
	{
		RemoveItems(WANMEI_YINPIAO_ID, req.yinpiao, S2C::DROP_TYPE_TRADEAWAY, false);
	}
	//????????
	if (req.yinpiao + money_to_yinpiao)
	{
		element_data::item_tag_t tag = {element_data::IMT_NULL, 0};
		item_data *data = world_manager::GetDataMan().generate_item_from_player(WANMEI_YINPIAO_ID, &tag, sizeof(tag));
		if (data == NULL)
		{
			ASSERT(false);
			return;
		}
		data->count = req.yinpiao + money_to_yinpiao;
		int rst = inv.Push(*data);
		if (rst < 0 || data->count)
		{
			ASSERT(false);
			FreeItem(data);
			return;
		}
		pImp->_runner->obtain_item(WANMEI_YINPIAO_ID, 0, req.yinpiao + money_to_yinpiao, inv[rst].count, IL_INVENTORY, rst);
		GLog::log(GLOG_INFO, "???%d(???)???????%d(???)???,?????%d?????,?????????????????%d???", pImp->_parent->ID.id, _parent->ID.id, req.yinpiao, money_to_yinpiao);
		FreeItem(data);
	}

	send_ls_msg(GetParent(), h1);
	pImp->_runner->get_player_money(pImp->GetMoney(), pImp->_money_capacity, _color_name);

	//???????????
	_runner->market_trade_success(pImp->GetParent()->ID.id);
	pImp->_runner->market_trade_success(GetParent()->ID.id);

	//??????????????????
	pImp->ReduceSaveTimer(100);
	ReduceSaveTimer(100);
	stall_website_sync_emit("update");
}
