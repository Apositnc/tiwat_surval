#include <graphics.h>
#include <string>
#include <vector>//STL内容 与数组十分相似 
#include <iostream>

int idx_current_anim = 0;
const int PLAYER_ANIM_NUM = 6;
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;

const int BUTTON_WIDTH = 192;
const int BUTTON_HEIGHT = 75;

//音效
#pragma comment(lib,"Winmm.lib")

bool is_game_start = false;
bool running = true;

IMAGE img_player_left[PLAYER_ANIM_NUM];
IMAGE img_player_right[PLAYER_ANIM_NUM];



//使图片的透明部分能够被正确渲染的函数
#pragma comment(lib,"MSIMG32.LIB")//链接这个库 里面是微软处理图像的一个库
inline void putimage_alpha(int x, int y, IMAGE* img)
{//内联函数声明  xy是被绘制的坐标 img是指向图片的指针
	int w = img->getwidth();
	int h = img->getheight();//获取图像的宽高
	AlphaBlend(GetImageHDC(NULL), x, y, w, h,
		GetImageHDC(img), 0, 0, w, h, { AC_SRC_OVER,0,255,AC_SRC_ALPHA });
}


//加载图片
class Animation
{
public:

	Animation(LPCTSTR path,int num,int interval)
	{
		interval_ms = interval;//每帧之间的时间间隔 单位为ms

		TCHAR path_file[256];//储存路径的变量
		for (int i = 0; i < num; i++)//使用for循环来生成每一帧的图片路径
		{
			_stprintf_s(path_file, path, i);

			IMAGE* frame = new IMAGE();
			loadimage(frame, path_file);
			frame_list.push_back(frame);
		}
	}
	~Animation()
	{
		for (size_t i = 0; i < frame_list.size(); i++)
			delete frame_list[i];
	}

	void Play(int x, int y, int delta)
	{
		timer += delta;
		if (timer >= interval_ms)
		{
			idx_frame = (idx_frame + 1) % frame_list.size();
			timer = 0;
		}
		putimage_alpha(x, y, frame_list[idx_frame]);
	}
private:
	int timer = 0;//计时器
	int idx_frame = 0;//动画帧索引
	int interval_ms = 0;
	std::vector<IMAGE*> frame_list;
};//将动画帧序列的vector容器定义为私有成员

void LoadAnimation() {
	for (size_t i = 0; i < PLAYER_ANIM_NUM; i++)
	{
		std::wstring path = L"img/player_left_" + std::to_wstring(i) + L".png";
		loadimage(&img_player_left[i], path.c_str());
	}
	for (size_t i = 0; i < PLAYER_ANIM_NUM; i++)
	{
		std::wstring path = L"img/player_right_" + std::to_wstring(i) + L".png";
		loadimage(&img_player_right[i], path.c_str());
	}
}

class Player {
public:

	const int PLAYER_WIDTH = 80;
	const int PLAYER_HEIGHT = 80;

	Player()
	{
		loadimage(&img_shadow, _T("img/shadow_player.png"));//阴影 
		anim_left = new Animation(_T("img/player_left_%d.png"), 6, 45);
		anim_right = new Animation(_T("img/player_right_%d.png"), 6, 45);//调用函数
	}
	~Player()
	{
		delete anim_left;
		delete anim_right;
	}

	void ProcessEvent(const ExMessage& msg)
	{
		if (msg.message == WM_KEYDOWN)
		{
			switch (msg.vkcode)
			{
			case VK_UP:
				is_move_up = true;
				break;
			case VK_DOWN:
				is_move_down = true;
				break;
			case VK_LEFT:
				is_move_left = true;
				break;
			case VK_RIGHT:
				//player_pos.x += PLAYERR_SPEED; 直接使用这种放法会导致移动时出现卡顿 移动的距离不一样
				//直接使用bool类型变量来使其能够连续的丝滑移动
				is_move_right = true;
				break;
			}
		}
		else if (msg.message == WM_KEYUP)
		{
			switch (msg.vkcode)
			{
			case VK_UP:
				is_move_up = false;
				break;
			case VK_DOWN:
				is_move_down = false;
				break;
			case VK_LEFT:
				is_move_left = false;
				break;
			case VK_RIGHT:
				is_move_right = false;
				break;
			}
		}
	}
	void move()
	{
		//保持速度的稳定，确定如果同时按下两个按键，斜方向的速度也是稳定的
		//设计定的speed
		int dir_x = is_move_right - is_move_left;
		int dir_y = is_move_down - is_move_up;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0) {
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			player_pos.x += (int)(PLAYER_SPEED * normalized_x);
			player_pos.y += (int)(PLAYER_SPEED * normalized_y);
		}

		//不会超出边界
		if (player_pos.x < 0) player_pos.x = 0;
		if (player_pos.y < 0) player_pos.y = 0;
		if (player_pos.x + PLAYER_WIDTH > WINDOW_WIDTH) player_pos.x = WINDOW_WIDTH - PLAYER_WIDTH;
		if (player_pos.y + PLAYER_HEIGHT > WINDOW_HEIGHT) player_pos.y = WINDOW_HEIGHT - PLAYER_HEIGHT;

	}
	void Draw(int delta)
	{
		int po_shadow_x = player_pos.x + (PLAYER_WIDTH / 2 - SHADOW_WIDTH / 2);
		int po_shadow_y = player_pos.y + PLAYER_HEIGHT - 8;
		putimage_alpha(po_shadow_x, po_shadow_y, &img_shadow);

		static bool facing_left = false;
		int dir_x = is_move_right - is_move_left;
		if (dir_x < 0) {
			facing_left = true;
		}
		else if (dir_x > 0) {
			facing_left = false;
		}
		if (facing_left) {
			anim_left->Play(player_pos.x, player_pos.y, delta);
		}
		else{
			anim_right->Play(player_pos.x, player_pos.y, delta);//绘制向左或者向右
		}
	}

	const POINT& GetPosition() const
	{
		return player_pos;
	}

private:
	const int PLAYER_SPEED = 6;
	const int SHADOW_WIDTH = 32;
private:
	IMAGE img_shadow;
	Animation* anim_left;
	Animation* anim_right;
	POINT player_pos = { 500,500 };

	bool is_move_up = false;
	bool is_move_down = false;
	bool is_move_left = false;
	bool is_move_right = false;

	
};

class Bullet {
public:
	POINT position = { 0, 0 };
	Bullet() = default;
	~Bullet() = default;
	
	void Draw() const
	{
		setlinecolor(RGB(255, 155, 50));
		setfillcolor(RGB(200, 75, 10));
		fillcircle(position.x,position.y,RADIUS);
	}

private:
	const int RADIUS = 10;
};

class Enemy {
public:
	Enemy()
	{
		loadimage(&enemy_img_shadow, _T("img/shadow_enemy.png"));//阴影 
		enemy_left = new Animation(_T("img/enemy_left_%d.png"), 6, 45);
		enemy_right = new Animation(_T("img/enemy_right_%d.png"), 6, 45);//调用函数
		

		enum class SpawnEdge
		{
			Up = 0,
			Down,
			Left,
			Right

		};
		
		SpawnEdge edge = (SpawnEdge)(rand() % 4);
		switch (edge)
		{
		case SpawnEdge::Up:
			enemy_pos.x = rand() % WINDOW_WIDTH;
			enemy_pos.y = -ENEMY_HEIGHT;
			break;
		case SpawnEdge::Down:
			enemy_pos.x = rand() % WINDOW_WIDTH;
			enemy_pos.y = WINDOW_HEIGHT;
			break;
		case SpawnEdge::Left:
			enemy_pos.x = -ENEMY_WIDTH;
			enemy_pos.y = rand() % WINDOW_HEIGHT;
			break;
		case SpawnEdge::Right:
			enemy_pos.x = WINDOW_WIDTH;
			enemy_pos.y = rand() % WINDOW_HEIGHT;
			break;
		default:
			break;
		}
	
	}


	//使用指针避免不必要的拷贝， const 避免对参数修改
	bool CheckBulletCollision(const Bullet& bullet)
	{
		bool is_overlap_x = bullet.position.x >= enemy_pos.x && bullet.position.x <= enemy_pos.x + ENEMY_WIDTH;
		bool is_overlap_y = bullet.position.y >= enemy_pos.y && bullet.position.y <= enemy_pos.y + ENEMY_HEIGHT;
		return is_overlap_x && is_overlap_y;
	}

	bool CheckPlayerCollision(const Player& player)
	{
		//enemy的中心点
		POINT check_position = { enemy_pos.x + ENEMY_WIDTH / 2,enemy_pos.y + ENEMY_HEIGHT / 2 };
		const POINT& player_position = player.GetPosition();
		bool is_collision_x = check_position.x >= player_position.x && check_position.x <= player_position.x + ENEMY_WIDTH;
		bool is_collision_y = check_position.y >= player_position.y && check_position.y <= player_position.y + ENEMY_HEIGHT;
		return is_collision_x && is_collision_y;
	}

	void Move(const Player& player)
	{
		const POINT& player_position = player.GetPosition();
		int dir_x = player_position.x - enemy_pos.x;
		int dir_y = player_position.y - enemy_pos.y;
		double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
		if (len_dir != 0) {
			double normalized_x = dir_x / len_dir;
			double normalized_y = dir_y / len_dir;
			enemy_pos.x += (int)(ENEMY_SPEED * normalized_x);
			enemy_pos.y += (int)(ENEMY_SPEED * normalized_y);
		}

		if (player_position.x > enemy_pos.x) {
			// 如果玩家在敌人的右边，敌人应该面向右
			facing_left = false;
		}
		else {
			// 如果玩家在敌人的左边或同一水平线上，敌人应该面向左
			facing_left = true;
		}

	}

	void Draw(int delta)
	{
		int pos_shadow_x = enemy_pos.x + (ENEMY_WIDTH / 2 - ENEMY_SHADOW_WIDTH / 2);
		int pos_shadow_y = enemy_pos.y + ENEMY_HEIGHT - 35;
		putimage_alpha(pos_shadow_x, pos_shadow_y,&enemy_img_shadow);
		
		if (facing_left)
		{
			enemy_left->Play(enemy_pos.x, enemy_pos.y, delta);
		}
		else{
			enemy_right->Play(enemy_pos.x, enemy_pos.y, delta);
		}
	}

	void Hurt()
	{
		alive = false;
	}

	bool CheckAlive()
	{
		return alive;
	}

	~Enemy()
	{
		delete enemy_left;
		delete enemy_right;
	}

	

private:
	const int ENEMY_SPEED = 2;
	const int ENEMY_WIDTH = 80;
	const int ENEMY_HEIGHT = 80;
	const int ENEMY_SHADOW_WIDTH = 48;
private:
	IMAGE enemy_img_shadow;
	Animation* enemy_left;
	Animation* enemy_right;
	POINT enemy_pos = { 0,0 };

	bool facing_left = false;
	bool alive = true;

	bool is_move_up = false;
	bool is_move_down = false;
	bool is_move_left = false;
	bool is_move_right = false;


};

class Button
{
public:
	Button(RECT rect,LPCTSTR path_img_idle,LPCTSTR path_img_hovered,LPCTSTR path_img_pushed)
	{
		region = rect;

		loadimage(&img_idle, path_img_idle);
		loadimage(&img_hovered, path_img_hovered);
		loadimage(&img_pushed, path_img_pushed);
	}
	~Button() = default;

	void ProcessEvent(const ExMessage& msg)
	{
		switch (msg.message)
		{
		case WM_MOUSEMOVE:
			if (status == Status::Idle && CheckCursorHit(msg.x, msg.y))
				status = Status::Hovered;
			else if (status == Status::Hovered && !CheckCursorHit(msg.x, msg.y))
				status = Status::Idle;
			break;
		case WM_LBUTTONDOWN:
			if (CheckCursorHit(msg.x, msg.y))
			{
				status = Status::Pushed;
			}
			break;
		case WM_LBUTTONUP:
			if (status == Status::Pushed)
			{
				OnClick();
			}
			break;
		default:
			break;
		}
	}

	void Draw()
	{
		switch (status)
		{
		case Status::Idle:
			putimage(region.left, region.top, &img_idle);
			break;
		case Status::Hovered:
			putimage(region.left, region.top, &img_hovered);
			break;
		case Status::Pushed:
			putimage(region.left, region.top, &img_pushed);
			break;
		}
	}

protected:
	virtual void OnClick() = 0;


private:
	enum class Status
	{
		Idle = 0,
		Hovered,
		Pushed
	};

private:
	RECT region;
	IMAGE img_idle;
	IMAGE img_hovered;
	IMAGE img_pushed;
	Status status = Status::Idle;

private:
	//检测有没有出现在区域内
	bool CheckCursorHit(int x,int y)
	{
		return x >= region.left && x <= region.right && y >= region.top && y <= region.bottom;
	}
};
//生成敌人
void TryGenerateEnmy(std::vector<Enemy*>& enemy_list)
{
	const int INTERVAL = 100;
	static int counter = 0;
	if ((++counter) % INTERVAL == 0)
		enemy_list.push_back(new Enemy());
}
//更新子弹位置
void UpdatteBullet(std::vector<Bullet>& bullet_list,const Player& player)
{
	const double RADIAL_SPEED = 0.0045;
	const double TANGENT_SPEED = 0.0045;
	POINT player_position = player.GetPosition();
	double radian_interval = 2 * 3.14159 / bullet_list.size();
	double radius = 100 + 25 * sin(GetTickCount() * RADIAL_SPEED);
	for (size_t i = 0; i < bullet_list.size(); i++)
	{
		double radian = GetTickCount() * TANGENT_SPEED + radian_interval * i;
		bullet_list[i].position.x = player_position.x + player.PLAYER_WIDTH / 2 + (int)(radius * sin(radian));
		bullet_list[i].position.y = player_position.y + player.PLAYER_HEIGHT / 2 + (int)(radius * cos(radian));
	}
}

void DrawaPlayeScore(int score) {
	static TCHAR text[64];
	_stprintf_s(text, _T("当前玩家得分为：%d"), score);

	setbkmode(TRANSPARENT);
	settextcolor(RGB(255, 85, 185));
	outtextxy(10, 10, text);
}

//开始按钮
class StartButton : public Button
{
public:
	StartButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed){}
	~StartButton() = default;

protected:
	void OnClick()
	{
		is_game_start = true;
		mciSendString(_T("play bgm repeat from 0"), NULL, 0, NULL);//循环播放
	}
};
//结束按钮
class QuitButton : public Button
{
public:
	QuitButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
		:Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {}
	~QuitButton() = default;

protected:
	void OnClick()
	{
		running = false;
	}
};

int main() {
	initgraph(1280, 720);//窗口

	//bgm
	mciSendString(_T("open mus/bgm.mp3 alias bgm"), NULL, 0, NULL);
	mciSendString(_T("open mus/hit.wav alias hit"), NULL, 0, NULL);
	
	int score = 0;
	Player p1;
	ExMessage msg;
	IMAGE img_background;
	IMAGE img_menu;
	std::vector<Enemy*> enemy_list;
	std::vector<Bullet> bullet_list(3);

	RECT start_btn, quit_btn;

	start_btn.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
	start_btn.right = start_btn.left + BUTTON_WIDTH;
	start_btn.top = 430;
	start_btn.bottom = start_btn.top + BUTTON_HEIGHT;

	quit_btn.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
	quit_btn.right = quit_btn.left + BUTTON_WIDTH;
	quit_btn.top = 550;
	quit_btn.bottom = quit_btn.top + BUTTON_HEIGHT;

	StartButton btn_start = StartButton(start_btn,
		_T("img/ui_start_idle.png"),_T("img/ui_start_hovered.png"), _T("img/ui_start_pushed.png"));
	QuitButton btn_quit = QuitButton(quit_btn,
		_T("img/ui_quit_idle.png"), _T("img/ui_quit_hovered.png"), _T("img/ui_quit_pushed.png"));

	loadimage(&img_menu,_T("img/menu.png"));
	loadimage(&img_background, _T("img/background.png"));//加载背景和阴影
	
	LoadAnimation();
	BeginBatchDraw();
	
	while (running)
	{
		DWORD start_time = GetTickCount();
		
		while (peekmessage(&msg))
		{
			if(is_game_start){
				p1.ProcessEvent(msg);
			}
			else {
				btn_start.ProcessEvent(msg);
				btn_quit.ProcessEvent(msg);
			}
		}
		p1.move();
		if (is_game_start) {
			TryGenerateEnmy(enemy_list);
			for (Enemy* enemy : enemy_list)
				enemy->Move(p1);

			//敌人与玩家碰撞
			for (Enemy* enemy : enemy_list)
			{
				if (enemy->CheckPlayerCollision(p1))
				{
					static TCHAR text[128];
					_stprintf_s(text, _T("玩家最终得分为：%d"), score);
					MessageBox(GetHWnd(), _T("扣1观看战败CG"), _T("游戏结束"), MB_OK);
					running = false;
					break;
				}
			}
			//敌人与子弹进行碰撞
			for (Enemy* enemy : enemy_list) {
				for (const Bullet& bullet : bullet_list)
				{
					if (enemy->CheckBulletCollision(bullet))
					{
						enemy->Hurt();
					}
				}
			}
			//移除生命值为0的敌人
			for (size_t i = 0; i < enemy_list.size(); i++)
			{
				Enemy* enemy = enemy_list[i];
				if (!enemy->CheckAlive())
				{

					mciSendString(_T("play hit from 0"), NULL, 0, NULL);
					std::swap(enemy_list[i], enemy_list.back());
					enemy_list.pop_back();//swap 和 pop 从vector中 删除元素
					delete enemy;//检测到要删除的元素时，先与最后一个元素进行交换，再popback，最后delete
					score++;
				}
			}

			UpdatteBullet(bullet_list, p1);
		}
			cleardevice();

			if (is_game_start)
			{
				putimage(0, 0, &img_background);

				p1.Draw(1000 / 144);
				for (Enemy* enemy : enemy_list)
					enemy->Draw(1000 / 144);
				for (const Bullet& bullet : bullet_list)
					bullet.Draw();
				DrawaPlayeScore(score);
			}
			else
			{
				putimage(0 , 0 , &img_menu);
				btn_start.Draw();
				btn_quit.Draw();
			}
			FlushBatchDraw();
		
		
		DWORD end_time = GetTickCount();//获取运行时间 计算什么时候休眠
		DWORD delta_time = end_time - start_time;
		
		if (delta_time < 1000 / 144)
		{
			Sleep(1000 / 144 - delta_time);
		}
	}
	EndBatchDraw();

	return 0;
}


//将角色和敌人分别封装
/*
玩家定位player
*/