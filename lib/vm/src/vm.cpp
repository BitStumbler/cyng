/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2017 Sylko Olzscher 
 * 
 */ 

#include <cyng/vm/vm.h>
#include <cyng/vm/memory.h>
#include <cyng/vm/context.h>
#include <cyng/intrinsics/traits/tag.hpp>
#include <cyng/core/class_interface.h>
#include <cyng/value_cast.hpp>
#include <cyng/factory.h>
#ifdef _DEBUG
#include <cyng/io/serializer.h>
#endif

#include <chrono>
#include <iomanip>

#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/process/environment.hpp>

namespace cyng 
{

	vm::vm(std::ostream& out, std::ostream& err)
	: tag_(boost::uuids::random_generator()())
	, out_(out)
	, err_(err)
	, stack_()
	, lib_()
	, error_register_()
	, cmp_register_(false)
	{}

	vm::vm(boost::uuids::uuid tag, std::ostream& out, std::ostream& err)
	: tag_(tag)
	, out_(out)
	, err_(err)
	, stack_()
	, lib_()
	, error_register_()
	, cmp_register_(false)
	{}

	boost::uuids::uuid vm::tag() const noexcept
	{
		return tag_;
	}
	
	void vm::run(vector_t&& vec)
	{
		if (vec.empty())	return;

#ifdef _DEBUG		
		auto now = std::chrono::system_clock::now();
#endif

#ifdef __DEBUG
		//
		//	It could be indicate a problem, if an instruction vector does not end with a REBA op,
		//	because call stack will not be restored properly.
		//
		const bool risk_flag = (vec.back().get_class().tag() != TC_CODE) 
			|| (vec.back().get_class().tag() == TC_CODE) && (value_cast(vec.back(), code::NOOP) != code::REBA);

		if (risk_flag)
		{
			using cyng::io::operator<<;
			std::stringstream ss;
			ss
				<< vec.size()
				<< " ops ending with "
				<< cyng::io::to_str(vec.back())
				;
			lib_.try_debug_log(*this, ss.str());
		}
#endif

		memory mem(std::move(vec));
		while (mem)
		{

#ifdef __DEBUG
			//stack_.dump(err_);

			//std::stringstream ss;
			//stack_.dump(ss);
			//lib_.try_debug_log(*this, ss.str());
#endif

			//
			//	next data or instruction
			//
			object obj = mem++;
			if (obj.get_class().tag() == TC_CODE)
			{
				//
				//	execute a single instruction
				//
				execute(value_cast(obj, code::NOOP), mem);
			}
			else 
			{
				//
				//	push value onto the stack
				//
				stack_.push(obj);
			}

#ifdef __DEBUG
			if (std::chrono::system_clock::now() - now > std::chrono::seconds(2))
			{
				std::cerr << "======> " << tag_ << ':' << std::setprecision(4) << mem.level() << "%" << std::endl;
			}
#endif
		}

#ifdef __DEBUG
		if (risk_flag)
		{
			using cyng::io::operator<<;
			std::stringstream ss;
			ss
				<< "stack dump: "
				;
			stack_.dump(ss);
			lib_.try_debug_log(*this, ss.str());
		}
#endif
#ifdef _DEBUG
		const auto delta = std::chrono::system_clock::now() - now;
		if (delta > std::chrono::seconds(2))
		{
			std::stringstream ss;
			ss
				<< tag_ 
				<< " *** "
				<< to_str(delta)
				<< " TIMEOUT ***"
				;

			const std::string msg = ss.str();
			context ctx(*this, mem, "log.msg.error");
			if (!lib_.try_error_log(ctx, msg))
			{
				std::cerr
					<< "\n\n"
					<< msg
					<< "\n\n"
					<< std::endl
					;
			}

		}
#endif
	}
	
	void vm::sync_run(vector_t&& prg)
	{
		//
		//	save and restore call stack
		//
		activation a(stack_);
		run(std::move(prg));
	}

	void vm::execute(code op, memory& mem)
	{
		switch (op)
		{
			case code::PUSH: 	
				//	push, mem[--sp] = mem[pc]
				stack_.push(mem++);
				break;
				
			case code::PC: 	//	push constant, mem[--sp] = x
// 				pc();
				break;
			case code::PR: 	//	push relative, mem[--sp] = mem[bp + s]
 				pr();
				break;
			case code::CORA: 	
				//	convert rel addr, mem[--sp] = (bp + s)
				break;
			case code::ASP: 	
				//	add to sp, sp = (sp + s)
				stack_.push(make_object());	//	push null on stack
				break;
			case code::CALL: 	
				//	call, mem[--sp] = pc; pc = x
				call(mem);
				break;

			case code::JA:
				jump_a(mem);
				break;
				// 			JCT = 7,	//!< 	jump count, if (--ct) pc = x
				// 			JP = 8,		//!< 	jump positive, if (mem[sp++] > 0) pc = x
				// 			JN = 9,		//!< 	jump negative, if (mem[sp++] < 0) pc = x
				// 			JZ = 0xA,	//!< 	jump zero, if (mem[sp++] == 0) pc = x
				// 			JNZ = 0xB,	//!< 	jump nonzero, if (mem[sp++] != 0) pc = x
				// 			JODD = 0xC,	//!< 	jump odd, if (mem[sp++] % 2 == 1) pc = x
				// 			JZON = 0xD,	//!< 	jump zero or neg, if (mem[sp++] <= 0) pc = x
				// 			JZOP = 0xE,	//!< 	jump zero or pos, if (mem[sp++] >= 0) pc = x
			case code::JE:	
				jump_error(mem);
				break;
			case code::JNE:
				jump_no_error(mem);
				break;

			case code::RET: 	
				//	return, pc = mem[sp++]
				break;
		
			case code::ADD: 	//	add, temp = mem[sp++]; mem[sp] = mem[sp] + temp; cy = carry
				break;
			case code::SUB: 	//	subtract, temp = mem[sp++]; mem[sp] = mem[sp] - temp
				break;
		
			case code::ESBA:	//	establish base address	
				//	mem[--sp] = bp; bp = sp;
				stack_.ebp();
				break;
			case code::REBA:	//	restore base address
				//	sp = bp; bp = mem[sp++];
				stack_.rbp();	//	resize stack
				break;
				
			case code::INVOKE: //	call a library function
				invoke(mem);
				break;
				
			case code::IDENT:	//	push VM tag onto stack
				stack_.push(make_object(tag_));
				break;

			case code::NOW:	//	push current timestamp on stack
				stack_.push(make_object(std::chrono::system_clock::now()));
				break;

			case code::PID:	//	push current process id on stack
				stack_.push(make_object(boost::this_process::get_id()));
				break;

				//
				//	assemble instructions
				//
			case code::ASSEMBLE_ATTR:
				stack_.assemble_attr();
				break;
			case code::ASSEMBLE_PARAM:
				stack_.assemble_param();
				break;
			case code::ASSEMBLE_ATTR_MAP:
				stack_.assemble_attr_map();
				break;
			case code::ASSEMBLE_PARAM_MAP:
				stack_.assemble_param_map();
				break;
			case code::ASSEMBLE_TUPLE:
				stack_.assemble_tuple();
				break;
			case code::ASSEMBLE_VECTOR:
				stack_.assemble_vector();
				break;
			case code::ASSEMBLE_SET:
				stack_.assemble_set();
				break;

			case code::LERR:
				//	load error register
				stack_.push(make_object(error_register_));
				break;
			case code::TSTERR:
				//	jump (true) if no error state is set
				cmp_register_ = !error_register_;
				break;
			case code::RESERR:
				error_register_.clear();
				break;

			case code::HALT: //	trigger halt
				//	set halt flag
				lib_.try_halt(*this);

				//	stop engine
				lib_.clear();
				break;
				
			case code::NOOP: //	no operation
				break;
			
			default:
				BOOST_ASSERT_MSG(false, "unknown op code");
				break;
		}
	}
	
	void vm::pr()
	{
		BOOST_ASSERT_MSG(!stack_.empty(), "stack is empty (PR)");
		BOOST_ASSERT_MSG(stack_.top().get_class().tag() == TC_UINT64, "wrong parameter type (PR)");
		const auto idx = value_cast<std::size_t>(stack_.top(), 0u);
		stack_.pop();
		stack_.setr(stack_.top(), idx);
		stack_.pop();
	}

	void vm::call(memory& mem)
	{
		//	Push the address that physically follows the call instruction
		//	onto the stack.
		//	call, mem[--sp] = pc; pc = x
		const auto addr = value_cast<std::size_t>(stack_.top(), 0u);
		stack_.push(make_object(mem.jump(addr)));
		stack_.pop();
	}
	
	void vm::jump_a(memory& mem)
	{
		//	jump always, pc = x
		const auto addr = value_cast<std::size_t>(stack_.top(), 0u);
		mem.jump(addr);
	}

	void vm::jump_error(memory& mem)
	{
		if (error_register_)
		{
			jump_a(mem);
		}
		else
		{
			//	drop jump address
			stack_.pop();
		}
	}

	void vm::jump_no_error(memory& mem)
	{
		if (!error_register_)
		{
			jump_a(mem);
		}
		else
		{
			//	drop jump address
			stack_.pop();
		}
	}

	void vm::ret(memory& mem)
	{
		// return, pc = mem[sp++]
		const auto addr = value_cast<std::size_t>(stack_.top(), 0u);
		mem.jump(addr);
		stack_.pop();
	}
	
	void vm::invoke(memory& mem)
	{
		BOOST_ASSERT_MSG(stack_.size() > 0, "missing parameter invoke()");
		const object obj = stack_.top();
		BOOST_ASSERT_MSG(obj.get_class().tag() == TC_STRING, "invoke requires a string with an function name");
		const std::string fname = value_cast< std::string >(obj, "---no function name---");
		stack_.pop();
	
		//
		//	call procedure
		//
		context ctx(*this, mem, fname);
		if (!lib_.invoke(fname, ctx))
		{
			std::stringstream ss;
			ss
				<< "***Warning: function ["
				<< fname
				<< "] is not registered in VM "
				<< tag_
				;

			const std::string msg = ss.str();
			if (!lib_.try_error_log(ctx, msg))
			{
				std::cerr
					<< "\n\n"
					<< msg
					<< "\n\n"
					<< std::endl
					;
			}

			//	set error register
// 			ctx.set_register(boost::system::errc::operation_canceled);
// 			return false;
		}
		
	}
	
}





