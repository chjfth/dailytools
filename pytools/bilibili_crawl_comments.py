#!/usr/bin/env python3
#coding: utf-8

# This program grabs user comments from a Bilibili video URL, and generates a local bilibili.comments.html .
#
# [2026-08-20] Code with 90% help from Deepseek.

import os,sys
import requests
import json
import time
import re
from urllib.parse import urlparse, parse_qs

VER_STR = '20260821.4'

# ---------- 配置区 ----------
#VIDEO_URL = "https://www.bilibili.com/video/BV1KxgH6wEj3"  # 你要爬取的视频链接，

USER_AGENT = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36"

COOKIE_FILE = r"d:\temp\Bilibili_user_cookie.txt"

# Sample user-login cookie. Will be re-read from file COOKIE_FILE.
COOKIE = "buvid_fp_plain=undefined; DedeUserID=2003585538; DedeUserID__ckMd5=dccd1862e52c9353; theme-tip-show=SHOWED; theme-avatar-tip-show=SHOWED; buvid3=3D77940D-EA35-DE56-892B-490092C6ED9D70172infoc; b_nut=1762171069; _uuid=BF848CA10-A119-D183-C1BC-F8F4FF52CF3481963infoc; LIVE_BUVID=AUTO5017631207625464; bsource_origin=toutiao_bilibilih5; timeMachine=0; rpdid=|(JlRYJ)kuuk0J'u~YlkJJR)R; home_feed_column=4; theme-switch-show=SHOWED; CURRENT_BLACKGAP=0; hit-dyn-v2=1; share_source_origin=COPY; fingerprint=45d2b91a8df9d86b38991ca692200610; buvid_fp=45d2b91a8df9d86b38991ca692200610; historyviewmode=grid; buvid4=07332335-ECBC-C189-91C0-3EF2CF1B8D3E74199-026070520-HCn2sElLRz/dJ/cTKuwgsw%3D%3D; bsource=share_source_copy_link; PVID=1; bili_ticket=eyJhbGciOiJIUzI1NiIsImtpZCI6InMwMyIsInR5cCI6IkpXVCJ9.eyJleHAiOjE3ODczMDgzMDksImlhdCI6MTc4NzA0OTA0OSwicGx0IjotMX0.OUpeH9LrVzMajRcjh6uNf97hQM3VaxqBkTbr4riaxWo; bili_ticket_expires=1787308249; SESSDATA=383ed8b6%2C1802680390%2C0f003%2A81CjAyCIo_0vg0xabXnrdCTVgqeb9JBH2a39QdJib_CS7tt1RBB_LBk03DyxjiI0AMmmYSVk9kTXAwQXdCRWJjWnJWNGN6VExPcGRiT0hZSnlEOTliOU94TzhMTXpkaDBIUEk1SVZTNUM2VG9GbzdmcHA3dF9jd2ZsRS1pa1Vpd3BRRzYxY0NSelRRIIEC; bili_jct=5aae5d8de89fc6c7b5f11d90ea8fe631; sid=5zury4ci; browser_resolution=1384-1623; CURRENT_QUALITY=80; bp_t_offset_2003585538=1238451882771349504; CURRENT_FNVAL=4048; b_lsid=FC553B2A_1A01E9E2EDE"  # 如果遇到风控，请填入你的B站登录Cookie（可选）

OUTPUT_HTML = "bilibili.comments.html"  # 输出的HTML文件名

SLEEP_SEC = 0.5

MAX_DEPTH = 2  # 最大抓取深度（Lv1=1, Lv2=2, Lv3=3）// 实际上 Lv3 并不能工作，抓到的是跟 Lv2 相同的内容。
# ---------------------------

REQUEST_PER_BATCH = 100
request_count = 0
import random
from datetime import datetime, timedelta, timezone

def requests_get(url, **kwargs):
	# Chj: This wrapper moderates HTTP requests sent to server, to avoid frequent-access banning.
	global request_count

	while True:
		resp = requests.get(url, **kwargs)
		
		if resp.status_code == 412:
			ban_delay_minutes = 10
			future = datetime.now() + timedelta(minutes=ban_delay_minutes)
			resume_on = future.strftime('%Y-%m-%d %H:%M:%S')
			
			print(f"Got Bilibili HTTP-412 banning, wait {ban_delay_minutes} minutes. Will Resume on {resume_on}.")
			
			time.sleep(60*ban_delay_minutes)

		else:
			request_count += 1

			if request_count % REQUEST_PER_BATCH == 0:
				delay_sec = 5 + random.randint(0, 10)
				print(f"HTTP requests reaches {request_count}, delay some random {delay_sec} seconds...")
				time.sleep(delay_sec)

			break
	
	return resp


def get_cid(bvid):
	"""根据BV号获取视频的cid（评论需要用）"""
	url = f"https://api.bilibili.com/x/web-interface/view?bvid={bvid}"
	headers = {"User-Agent": USER_AGENT}
	if COOKIE:
		headers["Cookie"] = COOKIE
	resp = requests_get(url, headers=headers)
	data = resp.json()
	if data["code"] != 0:
		raise Exception(f"获取cid失败: {data['message']}")
	return data["data"]["cid"]

def get_aid_and_cid(bvid):
	"""获取视频的 aid 和 cid"""
	url = f"https://api.bilibili.com/x/web-interface/view?bvid={bvid}"
	headers = {"User-Agent": USER_AGENT}
	if COOKIE:
		headers["Cookie"] = COOKIE
	resp = requests_get(url, headers=headers)
	data = resp.json()
	if data["code"] != 0:
		raise Exception(f"获取视频信息失败: {data['message']}")
	return data["data"]["aid"], data["data"]["cid"]

def fetch_comments(oid, root_id=0, page=1):
	"""
	抓取评论（支持一级评论和回复）
	oid: 视频的aid
	root_id: 0表示抓取一级评论，否则抓取对应根评论的回复
	"""
	# 关键：根据是否有 root 选择不同的端点
	if root_id == 0:
		url = "https://api.bilibili.com/x/v2/reply"
	else:
		url = "https://api.bilibili.com/x/v2/reply/reply"  # 正确的回复端点
	
	params = {
		"oid": oid,
		"type": 1,
		"pn": page,
		"ps": 20,          # 每页条数，可调
		"sort": 0
	}
	if root_id != 0:
		params["root"] = root_id
		# 可选：加上 web_location 参数（从cURL中看到）
		params["web_location"] = "333.788"
	
	headers = {
		"User-Agent": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/151.0.0.0 Safari/537.36",
		"Referer": "https://www.bilibili.com/video/BV1xV411374m/",
		"Origin": "https://www.bilibili.com",
		"Accept": "application/json, text/plain, */*",
		"Accept-Language": "zh-CN,zh;q=0.9,en-US;q=0.8,en;q=0.7",
		"Cookie": COOKIE,   # 请填入你从cURL中复制的完整Cookie
		"Sec-Fetch-Dest": "empty",
		"Sec-Fetch-Mode": "cors",
		"Sec-Fetch-Site": "same-site",
	}
	
	resp = requests_get(url, params=params, headers=headers)
	if resp.status_code != 200:
		print(f"HTTP错误: {resp.status_code}")
		return [], False
	
	data = resp.json()
	if data["code"] != 0:
		print(f"API错误: {data['message']} (code: {data['code']})")
		return [], False
	
	replies = data["data"].get("replies")
	if replies is None:
		replies = []
	
	# 判断是否有下一页
	page_info = data["data"].get("page", {})
	has_next = page_info.get("count", 0) > page * page_info.get("size", 20)
	return replies, has_next


def fetch_all_replies(aid, parent_rpid, depth=2):
	"""
	递归抓取指定评论的所有回复
	aid: 视频的aid（始终不变）
	parent_rpid: 父评论的rpid
	depth: 当前深度（Lv2=2, Lv3=3）
	"""
	all_replies = []
	page = 1
	
	while True:
		indent = "  " * (depth - 1)
		print(f"{indent}抓取第 {page} 页回复 (depth={depth})...")
		
		# 关键：oid始终是aid，root是父评论的rpid
		replies, has_next = fetch_comments(aid, root_id=parent_rpid, page=page)
		
		if not replies:
			print(f"{indent}此评论无回复")  # old: 当前页无回复
			break
		
		all_replies.extend(replies)
		print(f"{indent}当前页 {len(replies)} 条，累计 {len(all_replies)} 条")
		
		# 如果还没达到最大深度，递归抓取子回复
		if depth < MAX_DEPTH:
			for reply in replies:
				print(f"### rcount= {reply['rcount']}") # to-test
				if reply['rcount'] != '0': # rcount means reply-count
					child_replies = fetch_all_replies(aid, reply['rpid'], depth + 1)
					reply['children'] = child_replies
				else:
					print('no children here')
		else:
			# 达到最大深度，不再深入
			for reply in replies:
				reply['children'] = []
		
		if not has_next:
			break
		page += 1
		time.sleep(SLEEP_SEC)
	
	return all_replies

def build_html_tree(video_title, lv1_list, output_file):
	"""生成带三层缩进和层级序号的 HTML"""
	
	html_header = f"""<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>{video_title} - 评论对话树</title>
<style>
body {{ font-family: "Microsoft YaHei", sans-serif; max-width: 900px; margin: 20px auto; padding: 0 20px; background: #f7f9fc; }}
h1 {{ font-size: 22px; border-left: 5px solid #fb7299; padding-left: 15px; color: #222; }}
.comment-count {{ color: #555; }}
.comment {{ background: white; border-radius: 8px; padding: 14px 18px; margin: 12px 0; box-shadow: 0 1px 4px rgba(0,0,0,0.06); }}
.level-1 {{ margin-left: 0; }}
.level-2 {{ margin-left: 40px; border-left: 2px solid #e0e0e0; padding-left: 15px; }}
.level-3 {{ margin-left: 80px; border-left: 2px solid #f0f0f0; padding-left: 15px; background: #fafafa; }}
.user {{ font-weight: bold; color: #fb7299; }}
.time {{ font-size: 12px; color: #999; float: right; }}
.content {{ margin: 8px 0 0 0; line-height: 1.6; word-break: break-all; }}
.likes {{ font-size: 13px; color: #888; }}
.reply-to {{ font-size: 12px; color: #888; margin: 4px 0; }}
.seq-no {{ color: #bbb; font-size: 13px; margin-right: 8px; font-weight: normal; }}
hr {{ border: none; border-top: 1px solid #eee; margin: 18px 0; }}
</style>
</head>
<body>
<h1>📺 {video_title}</h1>
<p class="comment-count">共 <strong>{len(lv1_list)}</strong> 条主评论</p>
<hr>
"""
	html_body = ""
	
	def render_comment(comment, level=1, seq="", parent_name=None):
		"""递归渲染评论，seq 为当前评论的序号字符串，如 '2.3'"""
		user = comment.get('member', {}).get('uname', '未知用户')
		content = comment.get('content', {}).get('message', '').replace('\n', '<br>')
		like = comment.get('like', 0)
		ctime = time.strftime("%Y-%m-%d %H:%M", time.localtime(comment.get('ctime', 0)))
		
		# 生成引用信息
		reply_text = ""
		if parent_name:
			reply_text = f'<div class="reply-to">↳ 回复 @{parent_name}</div>'
		
		# 确定层级样式
		level_class = f"level-{min(level, 3)}"
		
		# 构建序号显示
		seq_display = f'<span class="seq-no">#{seq}</span>' if seq else ''
		
		html = f"""
<div class="comment {level_class}">
	{seq_display}<span class="user">{user}</span>
	<span class="time">{ctime}</span>
	{reply_text}
	<div class="content">{content}</div>
	<div class="likes">❤️ {like}</div>
"""
		
		# 处理子评论
		children = comment.get('children', [])
		if children and level < MAX_DEPTH:
			# 遍历子评论，并传入子序号
			for idx, child in enumerate(children, 1):
				child_seq = f"{seq}.{idx}" if seq else str(idx)
				html += render_comment(child, level + 1, child_seq, user)
		
		html += "</div>"
		return html
	
	# 渲染所有 Lv1 评论
	for idx, lv1 in enumerate(lv1_list, 1):
		# 传入 Lv1 序号
		html_body += render_comment(lv1, level=1, seq=str(idx))
		html_body += '<hr>'
	
	html_footer = "</body></html>"
	
	with open(output_file, "w", encoding="utf-8") as f:
		f.write(html_header + html_body + html_footer)

	print(f"✅ HTML已生成：{output_file}")


# 在 main() 函数开头，get_cid() 之前插入这段测试
def test_cookie_validity():
	"""测试当前Cookie是否能正常访问需要登录的接口"""
	test_url = "https://api.bilibili.com/x/web-interface/nav"  # 获取用户导航信息
	headers = {"User-Agent": USER_AGENT}
	if COOKIE:
		headers["Cookie"] = COOKIE
	resp = requests_get(test_url, headers=headers)
	data = resp.json()
	if data["code"] == 0:
		print(f"✅ Cookie有效！当前登录用户：{data['data']['uname']}")
		return True
	else:
		print(f"❌ Cookie无效或过期！错误码：{data['code']}，信息：{data['message']}")
		return False


def debug_comment_api(cid):
	"""模拟浏览器行为，查看完整响应"""
	import time
	
	# 方式1：使用我们脚本的参数
	url = "https://api.bilibili.com/x/v2/reply"
	params = {
		"oid": cid,
		"type": 1,
		"pn": 1,
		"ps": 20,
		"sort": 0
	}
	headers = {
		"User-Agent": USER_AGENT,
		"Cookie": COOKIE,
		"Referer": "https://www.bilibili.com/"
	}
	
	print(f"🛠️ 请求URL: {url}?{'&'.join([f'{k}={v}' for k,v in params.items()])}")
	
	resp = requests_get(url, params=params, headers=headers)
	data = resp.json()
	
	print(f"📦 响应状态码: {data['code']}")
	print(f"📝 响应消息: {data['message']}")
	print(f"📊 数据内容: {json.dumps(data, ensure_ascii=False, indent=2)}")
	
	return data


def main():
	
	if len(sys.argv)<2:
		print('需要一个 Bilibili URL 参数. 形如 https://www.bilibili.com/video/BV1KxgH6wEj3')
		exit(1)
	
	#VIDEO_URL = 'https://www.bilibili.com/video/BV1xV411374m' # sys.argv[1]
	VIDEO_URL = sys.argv[1]
	print(f'Using URL: {VIDEO_URL}')
	
	global COOKIE
	COOKIE = open(COOKIE_FILE).read()
	
	# 1. 从URL中提取BV号
	bvid = None
	if "BV" in VIDEO_URL:
		# 简单提取：取"BV"及其后的11位字符
		match = re.search(r'BV[a-zA-Z0-9]{10,11}', VIDEO_URL)
		if match:
			bvid = match.group()
	if not bvid:
		raise ValueError("未从URL中提取到BV号，请检查输入")

	if not test_cookie_validity():
		print("请重新从浏览器复制最新的Cookie")
		return

	print(f"🔍 正在获取视频信息... BV号: {bvid}")
	aid, cid = get_aid_and_cid(bvid)
	print(f"✅ aid: {aid} , cid: {cid}")

	#debug_comment_api(aid) # This is OK
	#debug_comment_api(cid) # This gets -404

	# 获取视频标题用于展示
	headers = {"User-Agent": USER_AGENT}
	if COOKIE:
		headers["Cookie"] = COOKIE
	view_url = f"https://api.bilibili.com/x/web-interface/view?bvid={bvid}"
	title = requests_get(view_url, headers=headers).json()["data"]["title"]
	print(f"📺 视频标题: {title}")

	# 第一步：抓取所有Lv1评论
	print("\n📄 开始抓取Lv1评论...")
	lv1_list = []
	page = 1
	while True:
		print(f"  抓取第 {page} 页Lv1评论...")
		comments, has_next = fetch_comments(aid, root_id=0, page=page)
		if not comments:
			print("  当前页无评论")
			break
		lv1_list.extend(comments)
		print(f"  当前页 {len(comments)} 条，累计 {len(lv1_list)} 条")
		if not has_next:
			break
		page += 1
		time.sleep(SLEEP_SEC)
	
	print(f"\n✅ 共抓取 {len(lv1_list)} 条Lv1评论")
	
	# 第二步：对每条Lv1评论，抓取其回复树
	print(f"\n🔄 开始抓取回复（最多{MAX_DEPTH}层）...")
	total_replies = 0
	for idx, lv1 in enumerate(lv1_list, 1):

		if lv1['rcount']==0: # reply_count==0
			continue

		print(f"\n  处理第 {idx}/{len(lv1_list)} 条主评论 (rpid: {lv1['rpid']})")
		# 关键：oid始终是aid，root是lv1的rpid
		lv1['children'] = fetch_all_replies(aid, lv1['rpid'], depth=2)
		total_replies += len(lv1['children'])
		time.sleep(SLEEP_SEC)
	
	print(f"\n✅ 共抓取 {total_replies} 条回复")
	
	# 第三步：生成HTML
	print(f"\n📝 生成HTML...")
	build_html_tree(title, lv1_list, OUTPUT_HTML)
	print("🎉 完成！")

if __name__ == "__main__":
	
	main()
