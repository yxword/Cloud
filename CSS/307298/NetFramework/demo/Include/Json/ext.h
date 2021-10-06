//
//  "$Id$"
//
//  Copyright (c)1992-2012, ZheJiang Dahua Technology Stock CO.LTD.
//  All Rights Reserved.
//
//	Description:	Json ��չ��֧�� WIN32 DLL
//	Revisions:		Year-Month-Day  SVN-Author  Modification
//

#ifndef DAHUA_JSON_EXTERNSION_H__
#define DAHUA_JSON_EXTERNSION_H__


#include <stddef.h>
#include "forwards.h"


namespace Json {


////////////////////////////////////////////////////////////////////////////////

/// FastWriter��װ�֧࣬�� WIN32 DLL
class JSON_API FastWriterWrapper
{
	FastWriterWrapper(FastWriterWrapper const&);
	FastWriterWrapper& operator=(FastWriterWrapper const&);

public:
	/// ���캯��
	FastWriterWrapper(Json::Value const& root, size_t reserved = 1024);

	/// ��������
	~FastWriterWrapper();

	/// ת������ַ����׵�ַ
	const char* data() const;

	/// ת������ַ�������
	size_t size() const;

private:
	struct FastWriterWrapperInternal;
	FastWriterWrapperInternal* m_internal;
};

////////////////////////////////////////////////////////////////////////////////

/// StyledWriter��װ�֧࣬�� WIN32 DLL
class JSON_API StyledWriterWrapper
{
	StyledWriterWrapper(StyledWriterWrapper const&);
	StyledWriterWrapper& operator=(StyledWriterWrapper const&);

public:
	/// ���캯��
	StyledWriterWrapper(Json::Value const& root, size_t reserved = 1024);

	/// ��������
	~StyledWriterWrapper();

	/// ת������ַ����׵�ַ
	const char* data() const;

	/// ת������ַ�������
	size_t size() const;

private:
	struct StyledWriterWrapperInternal;
	StyledWriterWrapperInternal* m_internal;
};


////////////////////////////////////////////////////////////////////////////////

/// Reader��װ�֧࣬�� WIN32 DLL
class JSON_API ReaderWrapper
{
	ReaderWrapper(ReaderWrapper const&);
	ReaderWrapper& operator=(ReaderWrapper const&);

public:
	/// ���캯��
	ReaderWrapper();

	/// ��������
	~ReaderWrapper();

	/// \brief Read a Value from a <a HREF="http://www.json.org">JSON</a> document.
	/// \param document UTF-8 encoded string containing the document to read.
	/// \param root [out] Contains the root value of the document if it was
	///             successfully parsed.
	/// \param collectComments \c true to collect comment and allow writing them back during
	///                        serialization, \c false to discard comments.
	///                        This parameter is ignored if Features::allowComments_
	///                        is \c false.
	/// \return \c true if the document was successfully parsed, \c false if an error occurred.
	bool parse(const char *beginDoc, const char *endDoc,
			Value &root,
			bool collectComments = true);

	/// \brief Returns a user friendly string that list errors in the parsed document.
	/// \return Formatted error message with the list of errors with their location in
	///         the parsed document. An empty string is returned if no error occurred
	///         during parsing.
	///         ���ص�ָ�������µ��� parse ֮��ʧЧ
	const char* getFormattedErrorMessages() const;

private:
	struct ReaderWrapperInternal;
	ReaderWrapperInternal* m_internal;
};

////////////////////////////////////////////////////////////////////////////////

/// ȡ��Ա�б�������
class JSON_API MemberNames
{
	MemberNames(MemberNames const&);
	MemberNames& operator=(MemberNames const&);

public:
	/// ���캯��
	MemberNames(Json::Value const&);

	/// ��������
	~MemberNames();

	/// ȡ��Ա����
	size_t count() const;

	/// ȡ��Ա����
	char const* operator[](size_t index) const;

private:
	struct MemberNamesInternal;
	MemberNamesInternal* m_internal;
};

} // namespace Json

#endif // DAHUA_JSON_EXTERNSION_H__
