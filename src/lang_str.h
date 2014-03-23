#pragma once

#define STR_SELECT_QT_DIR	"请选择qt所在目录"
#define STR_SELECT_BB_DIR	"请选择blockchain数据库所在目录"
#define STR_APP_FALSIFIED	"程序被篡改，禁止使用!"

#define	STR_APP_SRC_URL		"程序源码:https://github.com/laybor/BtcSafeTransaction"
#define STR_AUTHOR_HOME		"作者网站:http://xingfeng.org"

#define STR_PUBKEY1_ERROR	"公钥1错误"
#define STR_PUBKEY2_ERROR	"公钥2错误"
#define STR_SAME_PUBKEYS	"存在相同的公钥"

#define STR_DEF_COMMENT_WILL_SELL	"销售单"
#define STR_DEF_COMMENT_WILL_BUY	"购物单"

#define	STR_SEND_MONEY_SMALL	 "发送金额过小"

#define STR_MULSIG_ADDR_EMPTY	 "担保地址为空"

#define STR_ASK_SEND_MONEY_TO_ADDR			"确实要从你钱包转[%s]给[%s]么？"
#define STR_LOG_SENDED_MONEY_TO_MUL_ADDR	"发送金额至担保地址成功，请发送你的公钥(和交易ID)给商家."
#define STR_TIP_PREFIX_TXID		"交易ID:"
#define STR_TIP_INPUT_PASS		"请输入密码"
#define STR_PREFIX_ERROR		"错误："
#define STR_LOG_SENDED_MONEY_FROM_MUL_ADDR	"已确认发款完毕,建议上blockchain.info查一下是否有0确认的交易"

#define STR_ERROR_TX_DATA_EMPTY					"交易数据为空"
#define STR_ERROR_TX_DATA_WITH_SRCTX_EMPTY		"交易数据为空"

#define STR_UNSPENT_TX_INPUT_TITLE		"未找到最新交易数据，请手工输入[交易ID,交易ID]"
#define	STR_TX_INPUT_EMPTY				"无法自动寻找交易ID，请手工复制"
#define	STR_UNSPENT_TX_EMPTY			"未找到交易数据，或钱已被转出了，请更新blockchain"

#define STR_REQUEST_RECV_ADDR_EMPTY		"收款地址错误"

#define STR_TX_REQUEST_BUILDED	 "交易(请求)数据已生成，请发送数据给对方，等待其确认转币."

#define STR_RECV_REQUEST_AMOUNT_SMALL	"收款金额过小."

#define STR_TIP_RECV_AMOUNT_NOT_EQUAL_BALANCE	"你要收款的金额(%s)与担保地址历史余额(%s)不相同，确定继续收款么？"

#define STR_TIP_BALANCE_NOT_EQUAL_FINAL_AMOUNT	"警告：余额(%s)有可能不等于欲转账金额(%s)"

#define STR_TIP_FIND_EXCEPTION_SEND_FROM \
	"警告，发现异常！\n" \
	"正常应该是从担保地址转账出去的，现在发现疑似要从非担保地址地址%s 转账出去，确认还要继续？"

#define STR_ASK_SEND_FROM_MUL_ADDR_OF_AMOUNT "你确认要从%s，转币%s给对方么？转完后就退不了款的哦!"

#define STR_TIP_CAN_NOT_WEB_QUERY_BALANCE	"无法查询你的币种余额"


#define STR_ASK_INSERT_MY_PUBKEY_TO_COMMENT		"是否加自己的公钥也加入备注中"

#define STR_CAN_NOT_FIND_TX_VOUT	 "无法读取vout，找不到相关数据，请确认你的数据是最新的/请确认担保地址、买家/商家模式是对的"

#define STR_STATE_CONNECTED "状态：已连接"
#define STR_STATE_STARTING	"状态：启动中"

#define STR_STATE_WAIT	"状态：等待连接"