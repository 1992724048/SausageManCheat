#pragma once
#include <Windows.h>
#include <DbgHelp.h>
#include <string>
#include <sstream>
#include <encode.h>
#include <exception>
#include <iomanip>
#include <App.h>

class ExceptionTranslator final : std::exception {
	using NtStatusConverter = ULONG(WINAPI*)(NTSTATUS status);

public:
	ExceptionTranslator(const unsigned int exception_code, const PEXCEPTION_POINTERS exception_info) : nt_module_handle{nullptr},
	                                                                                                   exception_code{exception_code},
	                                                                                                   message_buffer{nullptr},
	                                                                                                   exception_info{exception_info},
	                                                                                                   nt_status_to_dos_error{nullptr} {
		nt_module_handle = GetModuleHandleW(L"NTDLL.DL");
		if (nt_module_handle) {
			nt_status_to_dos_error = reinterpret_cast<NtStatusConverter>(GetProcAddress(nt_module_handle, "RtlNtStatusToDosError"));
		}
	}

	~ExceptionTranslator() override {
		if (message_buffer) {
			LocalFree(message_buffer);
			message_buffer = nullptr;
		}
	}

	[[nodiscard]] auto what() const noexcept -> const char* override {
		if (nt_status_to_dos_error) {
			const auto dos_error_code = nt_status_to_dos_error(exception_code);
			if (FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE,
			                   nt_module_handle,
			                   dos_error_code,
			                   0,
			                   LPSTR(&message_buffer),
			                   0,
			                   nullptr)) {
				return message_buffer;
			}
		}
		return "unknown_error";
	}

	[[nodiscard]] auto get_exception_info() const -> PEXCEPTION_POINTERS {
		return exception_info;
	}

	[[nodiscard]] auto get_exception_code() const -> unsigned int {
		return exception_code;
	}

private:
	HMODULE nt_module_handle;
	unsigned int exception_code;
	char* message_buffer;
	PEXCEPTION_POINTERS exception_info;
	NtStatusConverter nt_status_to_dos_error;

public:
	static auto translate_seh_to_ce(const unsigned int exception_code, const PEXCEPTION_POINTERS exception_info) -> void {
		throw ExceptionTranslator{exception_code, exception_info};
	}
};

class Crash {
	inline static std::filesystem::path path_;
public:
	static auto init(std::filesystem::path path) -> void {
		SetErrorMode(SEM_NOGPFAULTERRORBOX | SEM_FAILCRITICALERRORS | SEM_NOALIGNMENTFAULTEXCEPT);
		SymInitialize(GetCurrentProcess(), nullptr, TRUE);
		SymSetOptions(SYMOPT_LOAD_LINES | SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);
		SymSetSearchPathW(GetCurrentProcess(), L"srv*C:\\symbols*https://msdl.microsoft.com/download/symbols");
		SetUnhandledExceptionFilter(unhandled_exception_filter_);
	}

private:
	static auto WINAPI unhandled_exception_filter_(const PEXCEPTION_POINTERS p_exception_info) -> LONG {
		std::stringstream msg;
		const std::string dump_path = (path_ / "dump.dmp").string();

		const std::shared_ptr<void> h_file(CreateFileA(dump_path.data(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr),
			CloseHandle);
		if (h_file.get() != INVALID_HANDLE_VALUE) {
			MINIDUMP_EXCEPTION_INFORMATION dump_info;
			dump_info.ThreadId = GetCurrentThreadId();
			dump_info.ExceptionPointers = p_exception_info;
			dump_info.ClientPointers = TRUE;

			constexpr auto dump_type = static_cast<MINIDUMP_TYPE>(
				MiniDumpWithFullMemory |
				MiniDumpWithHandleData |
				MiniDumpWithUnloadedModules |
				MiniDumpWithProcessThreadData |
				MiniDumpWithTokenInformation |
				MiniDumpWithThreadInfo |
				MiniDumpWithFullMemoryInfo |
				MiniDumpWithPrivateReadWriteMemory |
				MiniDumpIgnoreInaccessibleMemory);

			MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), h_file.get(), dump_type, &dump_info, nullptr, nullptr);
		}

		char symbol_info[sizeof(SYMBOL_INFO) + MAX_SYM_NAME] = {0};
		const auto p_symbol = reinterpret_cast<PSYMBOL_INFO>(symbol_info);
		p_symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		p_symbol->MaxNameLen = MAX_SYM_NAME;
		p_symbol->Name[0] = '\0';

		DWORD64 displacement = 0;
		SymFromAddr(GetCurrentProcess(), p_exception_info->ContextRecord->Eip, &displacement, p_symbol);

		const ExceptionTranslator error_info(p_exception_info->ExceptionRecord->ExceptionCode, p_exception_info);

		msg << Encode::utf8_to_gbk("线程ID: ") << GetCurrentThreadId() << "\n";
		msg << Encode::utf8_to_gbk("错误码: ") << std::hex << p_exception_info->ExceptionRecord->ExceptionCode << "\n";
		msg << Encode::utf8_to_gbk("方法名: ") << p_symbol->Name << "\n";
		msg << Encode::utf8_to_gbk("错误类型: ") << error_info.what();
		msg << "\n------------------------\n";
		msg << std::hex << p_exception_info->ContextRecord->Eip << ":\n";
		msg << dump_registers(p_exception_info->ContextRecord) << "\n\n";


		MessageBoxA(nullptr, msg.str().data(), Encode::utf8_to_gbk("错误").data(), MB_OK | MB_ICONERROR);

		exit(p_exception_info->ExceptionRecord->ExceptionCode);
		return EXCEPTION_CONTINUE_SEARCH;
	}

	static auto dump_registers(const CONTEXT* context) -> std::string {
		std::stringstream ss;
		ss << std::hex << std::setfill('0');
		ss << "RAX=0x" << std::setw(16) << context->Eax << "\n";
		ss << "RBX=0x" << std::setw(16) << context->Ebx << "\n";
		ss << "RCX=0x" << std::setw(16) << context->Ecx << "\n";
		ss << "RDX=0x" << std::setw(16) << context->Edx << "\n";
		ss << "RSI=0x" << std::setw(16) << context->Esi << "\n";
		ss << "RDI=0x" << std::setw(16) << context->Edi << "\n";
		ss << "RBP=0x" << std::setw(16) << context->Ebp << "\n";
		ss << "RSP=0x" << std::setw(16) << context->Esp << "\n";
		ss << "RIP=0x" << std::setw(16) << context->Eip << "\n";
		ss << "EFLAGS=0x" << std::setw(8) << context->EFlags;
		return ss.str();
	}
};
