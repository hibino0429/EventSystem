#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <deque>
#include <unordered_map>
#include <time.h>

using namespace std;

class IReceiver
{
public:
	[[noreturn]] virtual void Action(const std::string& message) = 0;
};


class Command
{
public:
	explicit Command() noexcept
	{
		isLoop = false;
	}
public:
	[[noreturn]] virtual void Execute() = 0;
public:
	[[noreturn]] void SetReceiver(IReceiver* receiver)
	{
		receive = receiver;
	}
	virtual const bool IsLoop() const noexcept
	{
		return isLoop;
	}
	[[noreturn]] void SetIsLoopFlag(const bool isLoopFlag) noexcept
	{
		isLoop = isLoopFlag;
	}
	IReceiver& GetReceiver() const noexcept
	{
		return *receive;
	}
private:
	bool isLoop;
	IReceiver* receive;
};



class Component
{
public:
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw2D() = 0;
	virtual void Draw3D() = 0;
};


class Invoke : public Component
{
public:
	//!@brief 先頭に命令を追加
	[[noreturn]] void AddCommand(Command* command) noexcept
	{
		commands.emplace_back(std::move(command));
	}
	//!@brief 任意の位置に命令を追加
	[[noreturn]] void AddCommand(Command* command, const int number) noexcept
	{
		std::unique_ptr<Command> com(std::move(command));
		commands.insert(commands.begin() + number, std::move(com));
	}
	//!@brief 先頭の命令を削除する
	[[noreturn]] void DeleteCommand() noexcept
	{
		commands.pop_back();
	}
	//!@brief 任意の場所の命令を削除する
	[[noreturn]] void DeleteCommand(const int number) noexcept
	{
		commands.erase(commands.begin() + number);
	}
	//!@brief 命令があるかチェックする
	const bool HasCommand() const noexcept
	{
		return commands.empty();
	}
	//!@brief 実行する
	[[noreturn]] void Execute() noexcept
	{
		for (auto itr = commands.begin(); itr != commands.end();)
		{
			(*itr)->Execute();
			if (!(*itr)->IsLoop())
			{
				itr = commands.erase(itr);
			}
			else
			{
				++itr;
			}
		}
	}
public:
	[[noreturn]] void Initialize() noexcept override {}
	[[noreturn]] void Draw2D() noexcept override {}
	[[noreturn]] void Draw3D() noexcept override {}
	[[noreturn]] void Update() noexcept override
	{
		while (!HasCommand())
		{
			Execute();
		}
	}
private:
	std::vector<std::unique_ptr<Command>> commands;
};







class EventSystem : public Component
{
public:
	explicit EventSystem(Invoke* invoker,IReceiver* receiver = nullptr) noexcept
	{
		invoke = invoker;
		receive = receiver;
	}
	explicit EventSystem() noexcept
	{
		//invokerとIReceiverを取得する(Managerから)
	}
public:
	void Initialize() noexcept override {}
	void Update() noexcept override {}
	void Draw2D() noexcept override {}
	void Draw3D() noexcept override {}
public:
	template<class EventType, typename... TypeArgs>
	[[noreturn]] void Add(TypeArgs&&... args) noexcept
	{
		EventType* com = new EventType(std::forward<TypeArgs>(args)...);
		invoke->AddCommand(com);
		if (receive != nullptr)
		{
			com->SetReceiver(receive);
		}
	}
private:
	Invoke* invoke;
	IReceiver* receive;
};


class EventListener final : public Component
{
public:
	using KeyType = std::string;
	using ValueType = IReceiver;
	using Listener = std::unordered_map<KeyType, std::unique_ptr<ValueType>>;
	EventListener() = default;
	virtual ~EventListener() noexcept
	{
		listener.clear();
	}
public:
	void Initialize() noexcept override {}
	void Update() noexcept override {}
	void Draw2D() noexcept override {}
	void Draw3D() noexcept override {}
public:
	void Add(const KeyType& key, ValueType* receiver) noexcept
	{
		std::unique_ptr<ValueType> rec(receiver);
		listener[key] = std::move(rec);
	}
	template<class ListenerType, typename... TypeArgs>
	void Add(const KeyType& key, TypeArgs... args) noexcept
	{
		std::unique_ptr<ValueType> rec(make_unique<ListenerType>(std::forward<TypeArgs>(args)...));
		listener[key] = std::move(rec);
	}
	[[noreturn]] void Delete(const KeyType& key) noexcept
	{
		const auto& itr = listener.find(key);
		if (itr != listener.end())
		{
			listener.erase(itr);
		}
	}
	ValueType& Get(const KeyType& key) const noexcept
	{
		return *listener.at(key).get();
	}
private:
	Listener listener;
};


class Collision : public Command
{
public:
	Collision()
	{
		SetIsLoopFlag(false);
		id = 0;
	}
	Collision(const int idNum)
	{
		id = idNum;
	}
public:
	[[noreturn]] void Execute() noexcept override
	{
		cout << "ヒットしました id : " << id << endl;
	}
private:
	int id;
};









int main()
{
	// メモリリーク検出
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	
	Invoke* invoker = new Invoke();
	EventSystem* eSys = new EventSystem(invoker);
	
	int hit = 0;
	cout << "衝突しているか: true(1),false(0) : ";
	cin >> hit;
	if (hit == 1)
	{
		eSys->Add<Collision>(); //イベント追加
		//eSys->Add<Collision>(100);
	}

	while (!invoker->HasCommand())
	{
		invoker->Execute();
	}

	delete eSys;
	delete invoker;
	system("pause");
}