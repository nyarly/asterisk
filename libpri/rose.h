/*
 * libpri: An implementation of Primary Rate ISDN
 *
 * Copyright (C) 2009 Digium, Inc.
 *
 * Richard Mudgett <rmudgett@digium.com>
 *
 * See http://www.asterisk.org for more information about
 * the Asterisk project. Please do not directly contact
 * any of the maintainers of this project for assistance;
 * the project provides a web site, mailing lists and IRC
 * channels for your use.
 *
 * This program is free software, distributed under the terms of
 * the GNU General Public License Version 2 as published by the
 * Free Software Foundation. See the LICENSE file included with
 * this program for more details.
 *
 * In addition, when this program is distributed with Asterisk in
 * any form that would qualify as a 'combined work' or as a
 * 'derivative work' (but not mere aggregation), you can redistribute
 * and/or modify the combination under the terms of the license
 * provided with that copy of Asterisk, instead of the license
 * terms granted here.
 */

/*!
 * \file
 * \brief ROSE definitions and prototypes
 *
 * \details
 * This file contains all of the data structures and definitions needed
 * for ROSE component encoding and decoding.
 *
 * ROSE - Remote Operations Service Element
 * ASN.1 - Abstract Syntax Notation 1
 * APDU - Application Protocol Data Unit
 *
 * \author Richard Mudgett <rmudgett@digium.com>
 */

#ifndef _LIBPRI_ROSE_H
#define _LIBPRI_ROSE_H

#include <string.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------------- */


/* Northern Telecom DMS-100 RLT related operations */
#define ROSE_DMS100_RLT_SERVICE_ID		0x3e
#define ROSE_DMS100_RLT_OPERATION_IND	0x01
#define ROSE_DMS100_RLT_THIRD_PARTY		0x02

/*! \brief ROSE operation-value function code */
enum rose_operation {
	/*! \brief No ROSE operation */
	ROSE_None,
	/*! \brief Unknown OID/localValue operation-value code */
	ROSE_Unknown,

/* *INDENT-OFF* */
	/* ETSI Diversion-Operations */
	ROSE_ETSI_ActivationDiversion,          /*!< Invoke/Result */
	ROSE_ETSI_DeactivationDiversion,        /*!< Invoke/Result */
	ROSE_ETSI_ActivationStatusNotificationDiv,/*!< Invoke only */
	ROSE_ETSI_DeactivationStatusNotificationDiv,/*!< Invoke only */
	ROSE_ETSI_InterrogationDiversion,       /*!< Invoke/Result */
	ROSE_ETSI_DiversionInformation,         /*!< Invoke only */
	ROSE_ETSI_CallDeflection,               /*!< Invoke/Result */
	ROSE_ETSI_CallRerouting,                /*!< Invoke/Result */
	ROSE_ETSI_InterrogateServedUserNumbers, /*!< Invoke/Result */
	ROSE_ETSI_DivertingLegInformation1,     /*!< Invoke only */
	ROSE_ETSI_DivertingLegInformation2,     /*!< Invoke only */
	ROSE_ETSI_DivertingLegInformation3,     /*!< Invoke only */

	/*
	 * ETSI Advice-of-Charge-Operations
	 *
	 * Advice-Of-Charge-at-call-Setup(AOCS)
	 * Advice-Of-Charge-During-the-call(AOCD)
	 * Advice-Of-Charge-at-the-End-of-the-call(AOCE)
	 */
	ROSE_ETSI_ChargingRequest,              /*!< Invoke/Result */
	ROSE_ETSI_AOCSCurrency,                 /*!< Invoke only */
	ROSE_ETSI_AOCSSpecialArr,               /*!< Invoke only */
	ROSE_ETSI_AOCDCurrency,                 /*!< Invoke only */
	ROSE_ETSI_AOCDChargingUnit,             /*!< Invoke only */
	ROSE_ETSI_AOCECurrency,                 /*!< Invoke only */
	ROSE_ETSI_AOCEChargingUnit,             /*!< Invoke only */

	/* ETSI Explicit-Call-Transfer-Operations-and-Errors */
	ROSE_ETSI_EctExecute,                   /*!< Invoke/Result */
	ROSE_ETSI_ExplicitEctExecute,           /*!< Invoke/Result */
	ROSE_ETSI_RequestSubaddress,            /*!< Invoke only */
	ROSE_ETSI_SubaddressTransfer,           /*!< Invoke only */
	ROSE_ETSI_EctLinkIdRequest,             /*!< Invoke/Result */
	ROSE_ETSI_EctInform,                    /*!< Invoke only */
	ROSE_ETSI_EctLoopTest,                  /*!< Invoke/Result */

	/* ETSI Status-Request-Procedure */
	ROSE_ETSI_StatusRequest,                /*!< Invoke/Result */

	/* ETSI CCBS-Operations-and-Errors */
	ROSE_ETSI_CallInfoRetain,               /*!< Invoke only */
	ROSE_ETSI_CCBSRequest,                  /*!< Invoke/Result */
	ROSE_ETSI_CCBSDeactivate,               /*!< Invoke/Result */
	ROSE_ETSI_CCBSInterrogate,              /*!< Invoke/Result */
	ROSE_ETSI_CCBSErase,                    /*!< Invoke only */
	ROSE_ETSI_CCBSRemoteUserFree,           /*!< Invoke only */
	ROSE_ETSI_CCBSCall,                     /*!< Invoke only */
	ROSE_ETSI_CCBSStatusRequest,            /*!< Invoke/Result */
	ROSE_ETSI_CCBSBFree,                    /*!< Invoke only */
	ROSE_ETSI_EraseCallLinkageID,           /*!< Invoke only */
	ROSE_ETSI_CCBSStopAlerting,             /*!< Invoke only */

	/* ETSI CCBS-private-networks-Operations-and-Errors */
	ROSE_ETSI_CCBS_T_Request,               /*!< Invoke/Result */
	ROSE_ETSI_CCBS_T_Call,                  /*!< Invoke only */
	ROSE_ETSI_CCBS_T_Suspend,               /*!< Invoke only */
	ROSE_ETSI_CCBS_T_Resume,                /*!< Invoke only */
	ROSE_ETSI_CCBS_T_RemoteUserFree,        /*!< Invoke only */
	ROSE_ETSI_CCBS_T_Available,             /*!< Invoke only */

	/* ETSI CCNR-Operations-and-Errors */
	ROSE_ETSI_CCNRRequest,                  /*!< Invoke/Result */
	ROSE_ETSI_CCNRInterrogate,              /*!< Invoke/Result */

	/* ETSI CCNR-private-networks-Operations-and-Errors */
	ROSE_ETSI_CCNR_T_Request,               /*!< Invoke/Result */

	/* ETSI MCID-Operations */
	ROSE_ETSI_MCIDRequest,                  /*!< Invoke/Result */

	/* ETSI MWI-Operations-and-Errors */
	ROSE_ETSI_MWIActivate,                  /*!< Invoke/Result */
	ROSE_ETSI_MWIDeactivate,                /*!< Invoke/Result */
	ROSE_ETSI_MWIIndicate,                  /*!< Invoke only */

	/* Q.SIG Name-Operations */
	ROSE_QSIG_CallingName,                  /*!< Invoke only */
	ROSE_QSIG_CalledName,                   /*!< Invoke only */
	ROSE_QSIG_ConnectedName,                /*!< Invoke only */
	ROSE_QSIG_BusyName,                     /*!< Invoke only */

	/* Q.SIG SS-AOC-Operations */
	ROSE_QSIG_ChargeRequest,                /*!< Invoke/Result */
	ROSE_QSIG_GetFinalCharge,               /*!< Invoke only */
	ROSE_QSIG_AocFinal,                     /*!< Invoke only */
	ROSE_QSIG_AocInterim,                   /*!< Invoke only */
	ROSE_QSIG_AocRate,                      /*!< Invoke only */
	ROSE_QSIG_AocComplete,                  /*!< Invoke/Result */
	ROSE_QSIG_AocDivChargeReq,              /*!< Invoke only */

	/* Q.SIG Call-Transfer-Operations (CT) */
	ROSE_QSIG_CallTransferIdentify,         /*!< Invoke/Result */
	ROSE_QSIG_CallTransferAbandon,          /*!< Invoke only */
	ROSE_QSIG_CallTransferInitiate,         /*!< Invoke/Result */
	ROSE_QSIG_CallTransferSetup,            /*!< Invoke/Result */
	ROSE_QSIG_CallTransferActive,           /*!< Invoke only */
	ROSE_QSIG_CallTransferComplete,         /*!< Invoke only */
	ROSE_QSIG_CallTransferUpdate,           /*!< Invoke only */
	ROSE_QSIG_SubaddressTransfer,           /*!< Invoke only */

	ROSE_QSIG_PathReplacement,              /*!< Invoke only */

	/* Q.SIG Call-Diversion-Operations */
	ROSE_QSIG_ActivateDiversionQ,           /*!< Invoke/Result */
	ROSE_QSIG_DeactivateDiversionQ,         /*!< Invoke/Result */
	ROSE_QSIG_InterrogateDiversionQ,        /*!< Invoke/Result */
	ROSE_QSIG_CheckRestriction,             /*!< Invoke/Result */
	ROSE_QSIG_CallRerouting,                /*!< Invoke/Result */
	ROSE_QSIG_DivertingLegInformation1,     /*!< Invoke only */
	ROSE_QSIG_DivertingLegInformation2,     /*!< Invoke only */
	ROSE_QSIG_DivertingLegInformation3,     /*!< Invoke only */
	ROSE_QSIG_CfnrDivertedLegFailed,        /*!< Invoke only */

	/* Q.SIG SS-CC-Operations */
	ROSE_QSIG_CcbsRequest,                  /*!< Invoke/Result */
	ROSE_QSIG_CcnrRequest,                  /*!< Invoke/Result */
	ROSE_QSIG_CcCancel,                     /*!< Invoke only */
	ROSE_QSIG_CcExecPossible,               /*!< Invoke only */
	ROSE_QSIG_CcPathReserve,                /*!< Invoke/Result */
	ROSE_QSIG_CcRingout,                    /*!< Invoke only */
	ROSE_QSIG_CcSuspend,                    /*!< Invoke only */
	ROSE_QSIG_CcResume,                     /*!< Invoke only */

	/* Q.SIG SS-MWI-Operations */
	ROSE_QSIG_MWIActivate,                  /*!< Invoke/Result */
	ROSE_QSIG_MWIDeactivate,                /*!< Invoke/Result */
	ROSE_QSIG_MWIInterrogate,               /*!< Invoke/Result */

	/* Northern Telecom DMS-100 RLT related operations */
	/*! Invoke/Result: Must set invokeId to ROSE_DMS100_RLT_OPERATION_IND */
	ROSE_DMS100_RLT_OperationInd,
	/*! Invoke/Result: Must set invokeId to ROSE_DMS100_RLT_THIRD_PARTY */
	ROSE_DMS100_RLT_ThirdParty,

	ROSE_NI2_InformationFollowing,          /*!< Invoke only? */
	ROSE_NI2_InitiateTransfer,              /*!< Invoke only? Is this correct operation name? */

	ROSE_Num_Operation_Codes                /*!< Must be last in the enumeration */
/* *INDENT-ON* */
};

enum rose_error_code {
	/*! \brief No error occurred */
	ROSE_ERROR_None,
	/*! \brief Unknown OID/localValue error-value code */
	ROSE_ERROR_Unknown,

	/* General-Errors (ETS 300 196) and General-Error-List(Q.950) */
	ROSE_ERROR_Gen_NotSubscribed,	/*!< also: UserNotSubscribed */
	ROSE_ERROR_Gen_NotAvailable,
	ROSE_ERROR_Gen_NotImplemented,	/*!< Not in Q.950 */
	ROSE_ERROR_Gen_InvalidServedUserNr,
	ROSE_ERROR_Gen_InvalidCallState,
	ROSE_ERROR_Gen_BasicServiceNotProvided,
	ROSE_ERROR_Gen_NotIncomingCall,
	ROSE_ERROR_Gen_SupplementaryServiceInteractionNotAllowed,
	ROSE_ERROR_Gen_ResourceUnavailable,

	/* Additional General-Error-List(Q.950) */
	ROSE_ERROR_Gen_RejectedByNetwork,
	ROSE_ERROR_Gen_RejectedByUser,
	ROSE_ERROR_Gen_InsufficientInformation,
	ROSE_ERROR_Gen_CallFailure,
	ROSE_ERROR_Gen_ProceduralError,

	/* ETSI Diversion-Operations */
	ROSE_ERROR_Div_InvalidDivertedToNr,
	ROSE_ERROR_Div_SpecialServiceNr,
	ROSE_ERROR_Div_DiversionToServedUserNr,
	ROSE_ERROR_Div_IncomingCallAccepted,
	ROSE_ERROR_Div_NumberOfDiversionsExceeded,
	ROSE_ERROR_Div_NotActivated,
	ROSE_ERROR_Div_RequestAlreadyAccepted,

	/* ETSI Advice-of-Charge-Operations */
	ROSE_ERROR_AOC_NoChargingInfoAvailable,

	/* ETSI Explicit-Call-Transfer-Operations-and-Errors */
	ROSE_ERROR_ECT_LinkIdNotAssignedByNetwork,

	/* ETSI CCBS-Operations-and-Errors */
	ROSE_ERROR_CCBS_InvalidCallLinkageID,
	ROSE_ERROR_CCBS_InvalidCCBSReference,
	ROSE_ERROR_CCBS_LongTermDenial,
	ROSE_ERROR_CCBS_ShortTermDenial,
	ROSE_ERROR_CCBS_IsAlreadyActivated,
	ROSE_ERROR_CCBS_AlreadyAccepted,
	ROSE_ERROR_CCBS_OutgoingCCBSQueueFull,
	ROSE_ERROR_CCBS_CallFailureReasonNotBusy,
	ROSE_ERROR_CCBS_NotReadyForCall,

	/* ETSI CCBS-private-networks-Operations-and-Errors */
	ROSE_ERROR_CCBS_T_LongTermDenial,
	ROSE_ERROR_CCBS_T_ShortTermDenial,

	/* ETSI MWI-Operations-and-Errors */
	ROSE_ERROR_MWI_InvalidReceivingUserNr,
	ROSE_ERROR_MWI_ReceivingUserNotSubscribed,
	ROSE_ERROR_MWI_ControllingUserNotRegistered,
	ROSE_ERROR_MWI_IndicationNotDelivered,
	ROSE_ERROR_MWI_MaxNumOfControllingUsersReached,
	ROSE_ERROR_MWI_MaxNumOfActiveInstancesReached,

	/* Q.SIG from various specifications */
	ROSE_ERROR_QSIG_Unspecified,

	/* Q.SIG SS-AOC-Operations */
	ROSE_ERROR_QSIG_AOC_FreeOfCharge,

	/* Q.SIG Call-Transfer-Operations (CT) */
	ROSE_ERROR_QSIG_CT_InvalidReroutingNumber,
	ROSE_ERROR_QSIG_CT_UnrecognizedCallIdentity,
	ROSE_ERROR_QSIG_CT_EstablishmentFailure,

	/* Q.SIG Call-Diversion-Operations (Additional Q.SIG specific errors) */
	ROSE_ERROR_QSIG_Div_TemporarilyUnavailable,
	ROSE_ERROR_QSIG_Div_NotAuthorized,

	/* Q.SIG SS-CC-Operations */
	ROSE_ERROR_QSIG_ShortTermRejection,
	ROSE_ERROR_QSIG_LongTermRejection,
	ROSE_ERROR_QSIG_RemoteUserBusyAgain,
	ROSE_ERROR_QSIG_FailureToMatch,
	ROSE_ERROR_QSIG_FailedDueToInterworking,

	/* Q.SIG SS-MWI-Operations */
	ROSE_ERROR_QSIG_InvalidMsgCentreId,

	/* Northern Telecom DMS-100 RLT related operations */
	ROSE_ERROR_DMS100_RLT_BridgeFail,
	ROSE_ERROR_DMS100_RLT_CallIDNotFound,
	ROSE_ERROR_DMS100_RLT_NotAllowed,
	ROSE_ERROR_DMS100_RLT_SwitchEquipCongs,

	ROSE_ERROR_Num_Codes		/*!< Must be last in the enumeration */
};

#define ROSE_REJECT_BASE(base)		((base) * 0x100)
enum rose_reject_base {
	ROSE_REJECT_BASE_General,
	ROSE_REJECT_BASE_Invoke,
	ROSE_REJECT_BASE_Result,
	ROSE_REJECT_BASE_Error,

	/*! \brief Must be last in the list */
	ROSE_REJECT_BASE_Last
};

/*!
 * \brief From Facility-Information-Element-Components
 * {itu-t identified-organization etsi(0) 196 facility-information-element-component(3)}
 */
enum rose_reject_code {
	/*! \brief Not rejected */
	ROSE_REJECT_None = -1,
	/*! \brief Unknown reject code */
	ROSE_REJECT_Unknown = -2,

/* *INDENT-OFF* */
	ROSE_REJECT_Gen_UnrecognizedComponent       = ROSE_REJECT_BASE(ROSE_REJECT_BASE_General) + 0,
	ROSE_REJECT_Gen_MistypedComponent           = ROSE_REJECT_BASE(ROSE_REJECT_BASE_General) + 1,
	ROSE_REJECT_Gen_BadlyStructuredComponent    = ROSE_REJECT_BASE(ROSE_REJECT_BASE_General) + 2,

	ROSE_REJECT_Inv_DuplicateInvocation         = ROSE_REJECT_BASE(ROSE_REJECT_BASE_Invoke) + 0,
	ROSE_REJECT_Inv_UnrecognizedOperation       = ROSE_REJECT_BASE(ROSE_REJECT_BASE_Invoke) + 1,
	ROSE_REJECT_Inv_MistypedArgument            = ROSE_REJECT_BASE(ROSE_REJECT_BASE_Invoke) + 2,
	ROSE_REJECT_Inv_ResourceLimitation          = ROSE_REJECT_BASE(ROSE_REJECT_BASE_Invoke) + 3,
	ROSE_REJECT_Inv_InitiatorReleasing          = ROSE_REJECT_BASE(ROSE_REJECT_BASE_Invoke) + 4,
	ROSE_REJECT_Inv_UnrecognizedLinkedID        = ROSE_REJECT_BASE(ROSE_REJECT_BASE_Invoke) + 5,
	ROSE_REJECT_Inv_LinkedResponseUnexpected    = ROSE_REJECT_BASE(ROSE_REJECT_BASE_Invoke) + 6,
	ROSE_REJECT_Inv_UnexpectedChildOperation    = ROSE_REJECT_BASE(ROSE_REJECT_BASE_Invoke) + 7,

	ROSE_REJECT_Res_UnrecognizedInvocation      = ROSE_REJECT_BASE(ROSE_REJECT_BASE_Result) + 0,
	ROSE_REJECT_Res_ResultResponseUnexpected    = ROSE_REJECT_BASE(ROSE_REJECT_BASE_Result) + 1,
	ROSE_REJECT_Res_MistypedResult              = ROSE_REJECT_BASE(ROSE_REJECT_BASE_Result) + 2,

	ROSE_REJECT_Err_UnrecognizedInvocation      = ROSE_REJECT_BASE(ROSE_REJECT_BASE_Error) + 0,
	ROSE_REJECT_Err_ErrorResponseUnexpected     = ROSE_REJECT_BASE(ROSE_REJECT_BASE_Error) + 1,
	ROSE_REJECT_Err_UnrecognizedError           = ROSE_REJECT_BASE(ROSE_REJECT_BASE_Error) + 2,
	ROSE_REJECT_Err_UnexpectedError             = ROSE_REJECT_BASE(ROSE_REJECT_BASE_Error) + 3,
	ROSE_REJECT_Err_MistypedParameter           = ROSE_REJECT_BASE(ROSE_REJECT_BASE_Error) + 4,
/* *INDENT-ON* */
};


/* ------------------------------------------------------------------- */


/*
 * Q931InformationElement ::= [APPLICATION 0] IMPLICIT OCTET STRING
 */
struct roseQ931ie {
	/*!
	 * \brief The Q.931 ie is present if length is nonzero.
	 * (If this field is optional in the message.)
	 */
	u_int8_t length;

	/*!
	 * \brief We mostly just need to store the contents so we will defer
	 * decoding/encoding.
	 *
	 * \note To reduce the size of the structure, the memory for the
	 * ie contents is "allocated" after the structure.
	 * \note Remember the "allocated" memory needs to have room for a
	 * null terminator.
	 */
	unsigned char contents[0];
};

enum {
	/*! Bearer Capability has a max length of 12. */
	ROSE_Q931_MAX_BC = 12,
	/*! High Layer Compatibility has a max length of 5. */
	ROSE_Q931_MAX_HLC = 5,
	/*! Low Layer Compatibility has a max length of 18. */
	ROSE_Q931_MAX_LLC = 18,
	/*!
	 * User-User Information has a network dependent maximum.
	 * The network dependent maximum is either 35 or 131 octets
	 * in non-USER-INFORMATION messages.
	 */
	ROSE_Q931_MAX_USER = 131,
	/*!
	 * Progress Indicator has a max length of 4.
	 * There can be multiple progress indicator ies.
	 * Q.SIG allows up to 3.
	 * ITU-T allows up to 2.
	 */
	ROSE_Q931_MAX_PROGRESS = 3 * 4,
};


/* ------------------------------------------------------------------- */


/*
 * Comment obtained from ECMA-242 ASN.1 source.
 * a VisibleString containing:
 *   - the (local) date in 8 digits (YYYYMMDD),
 *   - followed by (local) time of day in 4 or 6 digits (HHMM[SS]),
 *   - optionally followed by the letter "Z" or
 *     by a local time differential in 5 digits ("+"HHMM or "-"HHMM);
 *     this date and time representation follows ISO 8601
 * Examples:
 * 1) 19970621194530, meaning 21 June 1997, 19:45:30;
 * 2) 19970621194530Z, meaning the same as 1);
 * 3) 19970621194530-0500, meaning the same as 1),
 *      5 hours retarded in relation to UTC time
 *
 * GeneralizedTime ::= [UNIVERSAL 24] IMPLICIT VisibleString
 */
struct roseGeneralizedTime {
	/*! GeneralizedTime (SIZE (12..19)) */
	unsigned char str[19 + 1];
};


/* ------------------------------------------------------------------- */


/*
 * PartyNumber ::= CHOICE {
 *     -- the numbering plan is the default numbering plan of
 *     -- the network. It is recommended that this value is
 *     -- used.
 *     unknownPartyNumber          [0] IMPLICIT NumberDigits,
 *
 *     -- the numbering plan is according to
 *     -- ITU-T Recommendation E.164.
 *     publicPartyNumber           [1] IMPLICIT PublicPartyNumber,
 *
 *     -- ATM endsystem address encoded as an NSAP address.
 *     nsapEncodedNumber           [2] IMPLICIT NsapEncodedNumber,
 *
 *     -- not used, value reserved.
 *     dataPartyNumber             [3] IMPLICIT NumberDigits,
 *
 *     -- not used, value reserved.
 *     telexPartyNumber            [4] IMPLICIT NumberDigits,
 *     privatePartyNumber          [5] IMPLICIT PrivatePartyNumber,
 *
 *     -- not used, value reserved.
 *     nationalStandardPartyNumber [8] IMPLICIT NumberDigits
 * }
 */
struct rosePartyNumber {
	/*!
	 * \brief Party numbering plan
	 * \details
	 * unknown(0),
	 * public(1) - The numbering plan is according to ITU-T E.164,
	 * nsapEncoded(2),
	 * data(3) - Reserved,
	 * telex(4) - Reserved,
	 * private(5),
	 * nationalStandard(8) - Reserved
	 */
	u_int8_t plan;

	/*!
	 * \brief Type-Of-Number valid for public and private party number plans
	 * \details
	 * public:
	 *  unknown(0),
	 *  internationalNumber(1),
	 *  nationalNumber(2),
	 *  networkSpecificNumber(3) - Reserved,
	 *  subscriberNumber(4) - Reserved,
	 *  abbreviatedNumber(6)
	 * \details
	 * private:
	 *  unknown(0),
	 *  level2RegionalNumber(1),
	 *  level1RegionalNumber(2),
	 *  pTNSpecificNumber/pISNSpecificNumber(3),
	 *  localNumber(4),
	 *  abbreviatedNumber(6)
	 */
	u_int8_t ton;

	/*! \brief Number present if length is nonzero. */
	u_int8_t length;

	/*! \brief Number string data. */
	unsigned char str[20 + 1];
};

/*
 * NumberScreened ::= SEQUENCE {
 *     partyNumber           PartyNumber,
 *     screeningIndicator    ScreeningIndicator
 * }
 */
struct roseNumberScreened {
	struct rosePartyNumber number;

	/*!
	 * \details
	 * userProvidedNotScreened(0),
	 * userProvidedVerifiedAndPassed(1),
	 * userProvidedVerifiedAndFailed(2) (Not used, value reserved),
	 * networkProvided(3)
	 */
	u_int8_t screening_indicator;
};

/*
 * PartySubaddress ::= CHOICE {
 *     -- not recommended
 *     UserSpecifiedSubaddress,
 *
 *     -- according to ITU-T Recommendation X.213
 *     NSAPSubaddress
 * }
 *
 * UserSpecifiedSubaddress ::= SEQUENCE {
 *     SubaddressInformation,
 *
 *     -- used when the coding of subaddress is BCD
 *     oddCountIndicator BOOLEAN OPTIONAL
 * }
 *
 * -- specified according to ITU-T Recommendation X.213. Some
 * -- networks may limit the subaddress value to some other
 * -- length, e.g. 4 octets
 * NSAPSubaddress ::= OCTET STRING (SIZE(1..20))
 *
 * -- coded according to user requirements. Some networks may
 * -- limit the subaddress value to some other length,
 * -- e.g. 4 octets
 * SubaddressInformation ::= OCTET STRING (SIZE(1..20))
 */
struct rosePartySubaddress {
	/*! \brief Subaddress type UserSpecified(0), NSAP(1) */
	u_int8_t type;

	/*! \brief Subaddress present if length is nonzero */
	u_int8_t length;

	union {
		/*! \brief Specified according to ITU-T Recommendation X.213 */
		unsigned char nsap[20 + 1];

		/*! \brief Use of this formatting is not recommended */
		struct {
			/*! \brief TRUE if OddCount present */
			u_int8_t odd_count_present;

			/*!
			 * \brief TRUE if odd number of BCD digits (optional)
			 * \note Used when the coding of subaddress is BCD.
			 */
			u_int8_t odd_count;
			unsigned char information[20 + 1];
		} user_specified;
	} u;
};

/*
 * Address ::= SEQUENCE {
 *     PartyNumber,
 *     PartySubaddress OPTIONAL
 * }
 */
struct roseAddress {
	struct rosePartyNumber number;

	/*! \brief Subaddress (Optional) */
	struct rosePartySubaddress subaddress;
};

/*
 * AddressScreened ::= SEQUENCE {
 *     PartyNumber,
 *     ScreeningIndicator,
 *     PartySubaddress OPTIONAL
 * }
 */
struct roseAddressScreened {
	struct rosePartyNumber number;

	/*! \brief Subaddress (Optional) */
	struct rosePartySubaddress subaddress;

	/*!
	 * \details
	 * userProvidedNotScreened(0),
	 * userProvidedVerifiedAndPassed(1),
	 * userProvidedVerifiedAndFailed(2) (Not used, value reserved),
	 * networkProvided(3)
	 */
	u_int8_t screening_indicator;
};

/*
 * PresentedNumberUnscreened ::= CHOICE {
 *     presentationAllowedNumber           [0] EXPLICIT PartyNumber,
 *     presentationRestricted              [1] IMPLICIT NULL,
 *     numberNotAvailableDueToInterworking [2] IMPLICIT NULL,
 *     presentationRestrictedNumber        [3] EXPLICIT PartyNumber
 * }
 */
struct rosePresentedNumberUnscreened {
	struct rosePartyNumber number;
	/*!
	 * \brief Number presentation type
	 * \details
	 * presentationAllowedNumber(0),
	 * presentationRestricted(1),
	 * numberNotAvailableDueToInterworking(2),
	 * presentationRestrictedNumber(3)
	 */
	u_int8_t presentation;
};

/*
 * PresentedNumberScreened ::= CHOICE {
 *     presentationAllowedNumber           [0] IMPLICIT NumberScreened,
 *     presentationRestricted              [1] IMPLICIT NULL,
 *     numberNotAvailableDueToInterworking [2] IMPLICIT NULL,
 *     presentationRestrictedNumber        [3] IMPLICIT NumberScreened
 * }
 */
struct rosePresentedNumberScreened {
	/*! \brief Screened number */
	struct roseNumberScreened screened;
	/*!
	 * \brief Number presentation type
	 * \details
	 * presentationAllowedNumber(0),
	 * presentationRestricted(1),
	 * numberNotAvailableDueToInterworking(2),
	 * presentationRestrictedNumber(3)
	 */
	u_int8_t presentation;
};

/*
 * PresentedAddressScreened ::= CHOICE {
 *     presentationAllowedAddress          [0] IMPLICIT AddressScreened,
 *     presentationRestricted              [1] IMPLICIT NULL,
 *     numberNotAvailableDueToInterworking [2] IMPLICIT NULL,
 *     presentationRestrictedAddress       [3] IMPLICIT AddressScreened
 * }
 */
struct rosePresentedAddressScreened {
	/*! \breif Screened address */
	struct roseAddressScreened screened;
	/*!
	 * \brief Address presentation type
	 * \details
	 * presentationAllowedAddress(0),
	 * presentationRestricted(1),
	 * numberNotAvailableDueToInterworking(2),
	 * presentationRestrictedAddress(3)
	 */
	u_int8_t presentation;
};


/* ------------------------------------------------------------------- */


/*
 * Time ::= SEQUENCE {
 *     lengthOfTimeUnit [1] IMPLICIT LengthOfTimeUnit,
 *     scale            [2] IMPLICIT Scale
 * }
 */
struct roseEtsiAOCTime {
	/*! LengthOfTimeUnit ::= INTEGER (0..16777215) -- 24 bit number */
	u_int32_t length;
	/*!
	 * \details
	 * oneHundredthSecond(0),
	 * oneTenthSecond(1),
	 * oneSecond(2),
	 * tenSeconds(3),
	 * oneMinute(4),
	 * oneHour(5),
	 * twentyFourHours(6)
	 */
	u_int8_t scale;
};

/*
 * Amount ::= SEQUENCE {
 *     currencyAmount [1] IMPLICIT CurrencyAmount,
 *     multiplier     [2] IMPLICIT Multiplier
 * }
 */
struct roseEtsiAOCAmount {
	/*! CurrencyAmount ::= INTEGER (0..16777215) -- 24 bit number */
	u_int32_t currency;
	/*!
	 * \details
	 * oneThousandth(0),
	 * oneHundredth(1),
	 * oneTenth(2),
	 * one(3),
	 * ten(4),
	 * hundred(5),
	 * thousand(6)
	 */
	u_int8_t multiplier;
};

/*
 * DurationCurrency ::= SEQUENCE {
 *     dCurrency       [1] IMPLICIT Currency,
 *     dAmount         [2] IMPLICIT Amount,
 *     dChargingType   [3] IMPLICIT ChargingType,
 *     dTime           [4] IMPLICIT Time,
 *     dGranularity    [5] IMPLICIT Time OPTIONAL
 * }
 */
struct roseEtsiAOCDurationCurrency {
	struct roseEtsiAOCAmount amount;
	struct roseEtsiAOCTime time;
	/*! \breif dGranularity (optional) */
	struct roseEtsiAOCTime granularity;
	/*! Currency ::= IA5String (SIZE (1..10)) -- Name of currency. */
	unsigned char currency[10 + 1];
	/*!
	 * \details
	 * continuousCharging(0),
	 * stepFunction(1)
	 */
	u_int8_t charging_type;
	/*! TRUE if granularity time is present */
	u_int8_t granularity_present;
};

/*
 * FlatRateCurrency ::= SEQUENCE {
 *     fRCurrency      [1] IMPLICIT Currency,
 *     fRAmount        [2] IMPLICIT Amount
 * }
 */
struct roseEtsiAOCFlatRateCurrency {
	struct roseEtsiAOCAmount amount;
	/*! Currency ::= IA5String (SIZE (1..10)) -- Name of currency. */
	unsigned char currency[10 + 1];
};

/*
 * VolumeRateCurrency ::= SEQUENCE {
 *     vRCurrency      [1] IMPLICIT Currency,
 *     vRAmount        [2] IMPLICIT Amount,
 *     vRVolumeUnit    [3] IMPLICIT VolumeUnit
 * }
 */
struct roseEtsiAOCVolumeRateCurrency {
	struct roseEtsiAOCAmount amount;
	/*! Currency ::= IA5String (SIZE (1..10)) -- Name of currency. */
	unsigned char currency[10 + 1];
	/*!
	 * \brief Volume rate volume unit
	 * \details
	 * octet(0),
	 * segment(1),
	 * message(2)
	 */
	u_int8_t unit;
};

/*
 * AOCSCurrencyInfo ::= SEQUENCE {
 *     chargedItem ChargedItem,
 *     CHOICE {
 *         specialChargingCode      SpecialChargingCode,
 *         durationCurrency         [1] IMPLICIT DurationCurrency,
 *         flatRateCurrency         [2] IMPLICIT FlatRateCurrency,
 *         volumeRateCurrency       [3] IMPLICIT VolumeRateCurrency
 *         freeOfCharge             [4] IMPLICIT NULL,
 *         currencyInfoNotAvailable [5] IMPLICIT NULL
 *     }
 * }
 */
struct roseEtsiAOCSCurrencyInfo {
	union {
		struct roseEtsiAOCDurationCurrency duration;
		struct roseEtsiAOCFlatRateCurrency flat_rate;
		struct roseEtsiAOCVolumeRateCurrency volume_rate;
		/*! SpecialChargingCode ::= INTEGER (1..10) */
		u_int8_t special_charging_code;
	} u;
	/*!
	 * \brief Determine what is stored in the union.
	 * \details
	 * specialChargingCode(0),
	 * durationCurrency(1),
	 * flatRateCurrency(2),
	 * volumeRateCurrency(3),
	 * freeOfCharge(4),
	 * currencyInfoNotAvailable(5),
	 */
	u_int8_t currency_type;
	/*!
	 * \brief What service is being billed.
	 * \details
	 * basicCommunication(0),
	 * callAttempt(1),
	 * callSetup(2),
	 * userToUserInfo(3),
	 * operationOfSupplementaryServ(4)
	 */
	u_int8_t charged_item;
};

/*
 * AOCSCurrencyInfoList ::= SEQUENCE SIZE (1..10) OF AOCSCurrencyInfo
 */
struct roseEtsiAOCSCurrencyInfoList {
	/*! \brief SEQUENCE SIZE (1..10) OF AOCSCurrencyInfo */
	struct roseEtsiAOCSCurrencyInfo list[10];

	/*! \brief Number of AOCSCurrencyInfo records present */
	u_int8_t num_records;
};

/*
 * RecordedCurrency ::= SEQUENCE {
 *     rCurrency       [1] IMPLICIT Currency,
 *     rAmount         [2] IMPLICIT Amount
 * }
 */
struct roseEtsiAOCRecordedCurrency {
	/*! Amount of currency involved. */
	struct roseEtsiAOCAmount amount;
	/*! Currency ::= IA5String (SIZE (1..10)) -- Name of currency. */
	unsigned char currency[10 + 1];
};

/*
 * RecordedUnits ::= SEQUENCE {
 *     CHOICE {
 *         recordedNumberOfUnits NumberOfUnits,
 *         notAvailable          NULL
 *     },
 *     recordedTypeOfUnits TypeOfUnit OPTIONAL
 * }
 */
struct roseEtsiAOCRecordedUnits {
	/*! \brief recordedNumberOfUnits INTEGER (0..16777215) -- 24 bit number */
	u_int32_t number_of_units;
	/*! \brief TRUE if number_of_units is not available (not present) */
	u_int8_t not_available;
	/*! \brief recordedTypeOfUnits INTEGER (1..16) (optional) */
	u_int8_t type_of_unit;
	/*! \brief TRUE if type_of_unit is present */
	u_int8_t type_of_unit_present;
};

/*
 * RecordedUnitsList ::= SEQUENCE SIZE (1..32) OF RecordedUnits
 */
struct roseEtsiAOCRecordedUnitsList {
	/*! \brief SEQUENCE SIZE (1..32) OF RecordedUnits */
	struct roseEtsiAOCRecordedUnits list[32];

	/*! \brief Number of RecordedUnits records present */
	u_int8_t num_records;
};

/*
 * ChargingAssociation ::= CHOICE {
 *     chargedNumber     [0] EXPLICIT PartyNumber,
 *     chargeIdentifier  ChargeIdentifier
 * }
 */
struct roseEtsiAOCChargingAssociation {
	/*! chargeIdentifier: INTEGER (-32768..32767) -- 16 bit number */
	int16_t id;
	/*! chargedNumber */
	struct rosePartyNumber number;
	/*!
	 * \details
	 * charge_identifier(0),
	 * charged_number(1)
	 */
	u_int8_t type;
};

/*
 * AOCECurrencyInfo ::= SEQUENCE {
 *     CHOICE {
 *         freeOfCharge [1] IMPLICIT NULL,
 *         specificCurrency SEQUENCE {
 *             recordedCurrency    [1] IMPLICIT RecordedCurrency,
 *             aOCEBillingId       [2] IMPLICIT AOCEBillingId OPTIONAL
 *         }
 *     },
 *     chargingAssociation ChargingAssociation OPTIONAL
 * }
 */
struct roseEtsiAOCECurrencyInfo {
	struct {
		/*! \brief recorded currency */
		struct roseEtsiAOCRecordedCurrency recorded;
		/*!
		 * \brief AOCEBillingId (optional)
		 * \details
		 * normalCharging(0),
		 * reverseCharging(1),
		 * creditCardCharging(2),
		 * callForwardingUnconditional(3),
		 * callForwardingBusy(4),
		 * callForwardingNoReply(5),
		 * callDeflection(6),
		 * callTransfer(7)
		 */
		u_int8_t billing_id;
		/*! \brief TRUE if billing id is present */
		u_int8_t billing_id_present;
	} specific;

	/*! \brief chargingAssociation (optional) */
	struct roseEtsiAOCChargingAssociation charging_association;

	/*! \brief TRUE if charging_association is present */
	u_int8_t charging_association_present;

	/*!
	 * \brief TRUE if this is free of charge.
	 * \note When TRUE, the contents of specific are not valid.
	 */
	u_int8_t free_of_charge;
};

/*
 * AOCEChargingUnitInfo ::= SEQUENCE {
 *     CHOICE {
 *         freeOfCharge [1] IMPLICIT NULL,
 *         specificChargingUnits SEQUENCE
 *         {
 *             recordedUnitsList [1] IMPLICIT RecordedUnitsList,
 *             aOCEBillingId     [2] IMPLICIT AOCEBillingId OPTIONAL
 *         }
 *     },
 *     chargingAssociation ChargingAssociation OPTIONAL
 * }
 */
struct roseEtsiAOCEChargingUnitInfo {
	/*! \brief Not valid if free_of_charge is TRUE */
	struct {
		/*! \brief RecordedUnitsList */
		struct roseEtsiAOCRecordedUnitsList recorded;
		/*!
		 * \brief AOCEBillingId (optional)
		 * \details
		 * normalCharging(0),
		 * reverseCharging(1),
		 * creditCardCharging(2),
		 * callForwardingUnconditional(3),
		 * callForwardingBusy(4),
		 * callForwardingNoReply(5),
		 * callDeflection(6),
		 * callTransfer(7)
		 */
		u_int8_t billing_id;
		/*! \brief TRUE if billing id is present */
		u_int8_t billing_id_present;
	} specific;

	/*! \brief chargingAssociation (optional) */
	struct roseEtsiAOCChargingAssociation charging_association;

	/*! \brief TRUE if charging_association is present */
	u_int8_t charging_association_present;

	/*!
	 * \brief TRUE if this is free of charge.
	 * \note When TRUE, the contents of specific are not valid.
	 */
	u_int8_t free_of_charge;
};

/*
 * ARGUMENT ChargingCase
 */
struct roseEtsiChargingRequest_ARG {
	/*!
	 * \details
	 * chargingInformationAtCallSetup(0),
	 * chargingDuringACall(1),
	 * chargingAtTheEndOfACall(2)
	 */
	u_int8_t charging_case;
};

/*
 * RESULT CHOICE {
 *     AOCSCurrencyInfoList,
 *     AOCSSpecialArrInfo,
 *     chargingInfoFollows NULL
 * }
 */
struct roseEtsiChargingRequest_RES {
	union {
		struct roseEtsiAOCSCurrencyInfoList currency_info;

		/*! AOCSSpecialArrInfo ::= INTEGER (1..10) */
		u_int8_t special_arrangement;
	} u;
	/*!
	 * \details
	 * currency_info_list(0),
	 * special_arrangement_info(1),
	 * charging_info_follows(2)
	 */
	u_int8_t type;
};

/*
 * ARGUMENT CHOICE {
 *     chargeNotAvailable NULL,
 *     AOCSCurrencyInfoList
 * }
 */
struct roseEtsiAOCSCurrency_ARG {
	struct roseEtsiAOCSCurrencyInfoList currency_info;
	/*!
	 * \details
	 * charge_not_available(0),
	 * currency_info_list(1)
	 */
	u_int8_t type;
};

/*
 * ARGUMENT CHOICE {
 *     chargeNotAvailable NULL,
 *     AOCSSpecialArrInfo
 * }
 */
struct roseEtsiAOCSSpecialArr_ARG {
	/*!
	 * \details
	 * charge_not_available(0),
	 * special_arrangement_info(1)
	 */
	u_int8_t type;
	/*! AOCSSpecialArrInfo ::= INTEGER (1..10) */
	u_int8_t special_arrangement;
};

/*
 * ARGUMENT CHOICE {
 *     chargeNotAvailable NULL,
 *     aOCDCurrencyInfo CHOICE {
 *         freeOfCharge [1] IMPLICIT NULL,
 *         specificCurrency SEQUENCE {
 *             recordedCurrency   [1] IMPLICIT RecordedCurrency,
 *             typeOfChargingInfo [2] IMPLICIT TypeOfChargingInfo,
 *             aOCDBillingId      [3] IMPLICIT AOCDBillingId OPTIONAL
 *         }
 *     }
 * }
 */
struct roseEtsiAOCDCurrency_ARG {
	struct {
		/*! \brief recorded currency */
		struct roseEtsiAOCRecordedCurrency recorded;
		/*!
		 * \brief Type of recorded charging information.
		 * \details
		 * subTotal(0),
		 * total(1)
		 */
		u_int8_t type_of_charging_info;
		/*!
		 * \brief AOCDBillingId (optional)
		 * \details
		 * normalCharging(0),
		 * reverseCharging(1),
		 * creditCardCharging(2)
		 */
		u_int8_t billing_id;
		/*! \brief TRUE if billing id is present */
		u_int8_t billing_id_present;
	} specific;
	/*!
	 * \details
	 * charge_not_available(0),
	 * free_of_charge(1),
	 * specific_currency(2)
	 */
	u_int8_t type;
};

/*
 * ARGUMENT CHOICE {
 *     chargeNotAvailable NULL,
 *     aOCDChargingUnitInfo CHOICE {
 *         freeOfCharge    [1] IMPLICIT NULL,
 *         specificChargingUnits SEQUENCE {
 *             recordedUnitsList  [1] IMPLICIT RecordedUnitsList,
 *             typeOfChargingInfo [2] IMPLICIT TypeOfChargingInfo,
 *             aOCDBillingId      [3] IMPLICIT AOCDBillingId OPTIONAL
 *         }
 *     }
 * }
 */
struct roseEtsiAOCDChargingUnit_ARG {
	struct {
		/*! \brief RecordedUnitsList */
		struct roseEtsiAOCRecordedUnitsList recorded;
		/*!
		 * \brief Type of recorded charging information.
		 * \details
		 * subTotal(0),
		 * total(1)
		 */
		u_int8_t type_of_charging_info;
		/*!
		 * \brief AOCDBillingId (optional)
		 * \details
		 * normalCharging(0),
		 * reverseCharging(1),
		 * creditCardCharging(2)
		 */
		u_int8_t billing_id;
		/*! \brief TRUE if billing id is present */
		u_int8_t billing_id_present;
	} specific;
	/*!
	 * \details
	 * charge_not_available(0),
	 * free_of_charge(1),
	 * specific_charging_units(2)
	 */
	u_int8_t type;
};

/*
 * ARGUMENT CHOICE {
 *     chargeNotAvailable NULL,
 *     AOCECurrencyInfo
 * }
 */
struct roseEtsiAOCECurrency_ARG {
	struct roseEtsiAOCECurrencyInfo currency_info;
	/*!
	 * \details
	 * charge_not_available(0),
	 * currency_info(1)
	 */
	u_int8_t type;
};

/*
 * ARGUMENT CHOICE {
 *     chargeNotAvailable NULL,
 *     AOCEChargingUnitInfo
 * }
 */
struct roseEtsiAOCEChargingUnit_ARG {
	struct roseEtsiAOCEChargingUnitInfo charging_unit;
	/*!
	 * \details
	 * charge_not_available(0),
	 * charging_unit(1)
	 */
	u_int8_t type;
};


/* ------------------------------------------------------------------- */


/*
 * ARGUMENT SEQUENCE {
 *     procedure           Procedure,
 *     basicService        BasicService,
 *     forwardedToAddress  Address,
 *     servedUserNr        ServedUserNr
 * }
 */
struct roseEtsiActivationDiversion_ARG {
	/*! \brief Forwarded to address */
	struct roseAddress forwarded_to;

	/*! \brief Forward all numbers if not present (Number length is zero). */
	struct rosePartyNumber served_user_number;

	/*! \details cfu(0), cfb(1), cfnr(2) */
	u_int8_t procedure;

	/*!
	 * \details
	 * allServices(0),
	 * speech(1),
	 * unrestrictedDigitalInformation(2),
	 * audio3k1Hz(3),
	 * unrestrictedDigitalInformationWithTonesAndAnnouncements(4),
	 * multirate(5),
	 * telephony3k1Hz(32),
	 * teletex(33),
	 * telefaxGroup4Class1(34),
	 * videotexSyntaxBased(35),
	 * videotelephony(36),
	 * telefaxGroup2-3(37),
	 * telephony7kHz(38),
	 * euroFileTransfer(39),
	 * fileTransferAndAccessManagement(40),
	 * videoconference(41),
	 * audioGraphicConference(42)
	 */
	u_int8_t basic_service;
};


/*
 * ARGUMENT SEQUENCE {
 *     procedure           Procedure,
 *     basicService        BasicService,
 *     servedUserNr        ServedUserNr
 * }
 */
struct roseEtsiDeactivationDiversion_ARG {
	/*! \brief Forward all numbers if not present (Number length is zero). */
	struct rosePartyNumber served_user_number;

	/*! \details cfu(0), cfb(1), cfnr(2) */
	u_int8_t procedure;

	/*!
	 * \details
	 * allServices(0),
	 * speech(1),
	 * unrestrictedDigitalInformation(2),
	 * audio3k1Hz(3),
	 * unrestrictedDigitalInformationWithTonesAndAnnouncements(4),
	 * multirate(5),
	 * telephony3k1Hz(32),
	 * teletex(33),
	 * telefaxGroup4Class1(34),
	 * videotexSyntaxBased(35),
	 * videotelephony(36),
	 * telefaxGroup2-3(37),
	 * telephony7kHz(38),
	 * euroFileTransfer(39),
	 * fileTransferAndAccessManagement(40),
	 * videoconference(41),
	 * audioGraphicConference(42)
	 */
	u_int8_t basic_service;
};


/*
 * ARGUMENT SEQUENCE {
 *     procedure           Procedure,
 *     basicService        BasicService,
 *     forwardedToAddresss Address,
 *     servedUserNr        ServedUserNr
 * }
 */
struct roseEtsiActivationStatusNotificationDiv_ARG {
	/*! \brief Forwarded to address */
	struct roseAddress forwarded_to;

	/*! \brief Forward all numbers if not present (Number length is zero). */
	struct rosePartyNumber served_user_number;

	/*! \details cfu(0), cfb(1), cfnr(2) */
	u_int8_t procedure;

	/*!
	 * \details
	 * allServices(0),
	 * speech(1),
	 * unrestrictedDigitalInformation(2),
	 * audio3k1Hz(3),
	 * unrestrictedDigitalInformationWithTonesAndAnnouncements(4),
	 * multirate(5),
	 * telephony3k1Hz(32),
	 * teletex(33),
	 * telefaxGroup4Class1(34),
	 * videotexSyntaxBased(35),
	 * videotelephony(36),
	 * telefaxGroup2-3(37),
	 * telephony7kHz(38),
	 * euroFileTransfer(39),
	 * fileTransferAndAccessManagement(40),
	 * videoconference(41),
	 * audioGraphicConference(42)
	 */
	u_int8_t basic_service;
};

/*
 * ARGUMENT SEQUENCE {
 *     procedure           Procedure,
 *     basicService        BasicService,
 *     servedUserNr        ServedUserNr
 * }
 */
struct roseEtsiDeactivationStatusNotificationDiv_ARG {
	/*! \brief Forward all numbers if not present (Number length is zero). */
	struct rosePartyNumber served_user_number;

	/*! \details cfu(0), cfb(1), cfnr(2) */
	u_int8_t procedure;

	/*!
	 * \details
	 * allServices(0),
	 * speech(1),
	 * unrestrictedDigitalInformation(2),
	 * audio3k1Hz(3),
	 * unrestrictedDigitalInformationWithTonesAndAnnouncements(4),
	 * multirate(5),
	 * telephony3k1Hz(32),
	 * teletex(33),
	 * telefaxGroup4Class1(34),
	 * videotexSyntaxBased(35),
	 * videotelephony(36),
	 * telefaxGroup2-3(37),
	 * telephony7kHz(38),
	 * euroFileTransfer(39),
	 * fileTransferAndAccessManagement(40),
	 * videoconference(41),
	 * audioGraphicConference(42)
	 */
	u_int8_t basic_service;
};


/*
 * ARGUMENT SEQUENCE {
 *     procedure           Procedure,
 *     basicService        BasicService DEFAULT allServices,
 *     servedUserNr        ServedUserNr
 * }
 */
struct roseEtsiInterrogationDiversion_ARG {
	/*! \brief Forward all numbers if not present (Number length is zero). */
	struct rosePartyNumber served_user_number;

	/*! \details cfu(0), cfb(1), cfnr(2) */
	u_int8_t procedure;

	/*!
	 * \details
	 * DEFAULT allServices
	 *
	 * \details
	 * allServices(0),
	 * speech(1),
	 * unrestrictedDigitalInformation(2),
	 * audio3k1Hz(3),
	 * unrestrictedDigitalInformationWithTonesAndAnnouncements(4),
	 * multirate(5),
	 * telephony3k1Hz(32),
	 * teletex(33),
	 * telefaxGroup4Class1(34),
	 * videotexSyntaxBased(35),
	 * videotelephony(36),
	 * telefaxGroup2-3(37),
	 * telephony7kHz(38),
	 * euroFileTransfer(39),
	 * fileTransferAndAccessManagement(40),
	 * videoconference(41),
	 * audioGraphicConference(42)
	 */
	u_int8_t basic_service;
};

/*
 * IntResult ::= SEQUENCE {
 *     servedUserNr        ServedUserNr,
 *     basicService        BasicService,
 *     procedure           Procedure,
 *     forwardedToAddress  Address
 * }
 */
struct roseEtsiForwardingRecord {
	/*! \brief Forwarded to address */
	struct roseAddress forwarded_to;

	/*! \brief Forward all numbers if not present (Number length is zero). */
	struct rosePartyNumber served_user_number;

	/*! \details cfu(0), cfb(1), cfnr(2) */
	u_int8_t procedure;

	/*!
	 * \details
	 * allServices(0),
	 * speech(1),
	 * unrestrictedDigitalInformation(2),
	 * audio3k1Hz(3),
	 * unrestrictedDigitalInformationWithTonesAndAnnouncements(4),
	 * multirate(5),
	 * telephony3k1Hz(32),
	 * teletex(33),
	 * telefaxGroup4Class1(34),
	 * videotexSyntaxBased(35),
	 * videotelephony(36),
	 * telefaxGroup2-3(37),
	 * telephony7kHz(38),
	 * euroFileTransfer(39),
	 * fileTransferAndAccessManagement(40),
	 * videoconference(41),
	 * audioGraphicConference(42)
	 */
	u_int8_t basic_service;
};

/*
 * roseInterrogationDiversion_RES
 * IntResultList ::= SET SIZE (0..29) OF IntResult
 */
struct roseEtsiForwardingList {
	/*!
	 * \brief SET SIZE (0..29) OF Forwarding Records
	 * \note Reduced the size of the array to conserve
	 * potential stack usage.
	 */
	struct roseEtsiForwardingRecord list[10];

	/*! \brief Number of Forwarding records present */
	u_int8_t num_records;
};

/*
 * ARGUMENT SEQUENCE {
 *     diversionReason         DiversionReason,
 *     basicService            BasicService,
 *     servedUserSubaddress    PartySubaddress OPTIONAL,
 *     callingAddress          [0] EXPLICIT PresentedAddressScreened OPTIONAL,
 *     originalCalledNr        [1] EXPLICIT PresentedNumberUnscreened OPTIONAL,
 *     lastDivertingNr         [2] EXPLICIT PresentedNumberUnscreened OPTIONAL,
 *     lastDivertingReason     [3] EXPLICIT DiversionReason OPTIONAL,
 *
 *     -- The User-user information element, as specified
 *     -- in ETS 300 102-1 [11], subclause 4.5.29, shall
 *     -- be embedded in the userInfo parameter.
 *     userInfo                Q931InformationElement OPTIONAL
 * }
 */
struct roseEtsiDiversionInformation_ARG {
	/*! \brief Served user subaddress (Optional) */
	struct rosePartySubaddress served_user_subaddress;

	/*! \brief Calling address (Optional) */
	struct rosePresentedAddressScreened calling;

	/*! \brief Original called number (Optional) */
	struct rosePresentedNumberUnscreened original_called;

	/*! \brief Last diverting number (Optional) */
	struct rosePresentedNumberUnscreened last_diverting;

	/*! \brief User-User information embedded in Q.931 IE (Optional) */
	struct roseQ931ie q931ie;
	/*! \brief q931ie.contents "allocated" after the stucture. */
	unsigned char q931ie_contents[ROSE_Q931_MAX_USER + 1];

	/*!
	 * \brief Last diverting reason (Optional)
	 *
	 * \details
	 * unknown(0),
	 * cfu(1),
	 * cfb(2),
	 * cfnr(3),
	 * cdAlerting(4),
	 * cdImmediate(5)
	 */
	u_int8_t last_diverting_reason;

	/*! \brief TRUE if CallingAddress is present */
	u_int8_t calling_present;

	/*! \brief TRUE if OriginalCalled is present */
	u_int8_t original_called_present;

	/*! \brief TRUE if LastDiverting is present */
	u_int8_t last_diverting_present;

	/*! \brief TRUE if LastDivertingReason is present */
	u_int8_t last_diverting_reason_present;

	/*!
	 * \details
	 * unknown(0),
	 * cfu(1),
	 * cfb(2),
	 * cfnr(3),
	 * cdAlerting(4),
	 * cdImmediate(5)
	 */
	u_int8_t diversion_reason;

	/*!
	 * \details
	 * allServices(0),
	 * speech(1),
	 * unrestrictedDigitalInformation(2),
	 * audio3k1Hz(3),
	 * unrestrictedDigitalInformationWithTonesAndAnnouncements(4),
	 * multirate(5),
	 * telephony3k1Hz(32),
	 * teletex(33),
	 * telefaxGroup4Class1(34),
	 * videotexSyntaxBased(35),
	 * videotelephony(36),
	 * telefaxGroup2-3(37),
	 * telephony7kHz(38),
	 * euroFileTransfer(39),
	 * fileTransferAndAccessManagement(40),
	 * videoconference(41),
	 * audioGraphicConference(42)
	 */
	u_int8_t basic_service;
};


/*
 * ARGUMENT SEQUENCE {
 *     deflectionAddress                 Address,
 *     presentationAllowedDivertedToUser PresentationAllowedIndicator OPTIONAL
 * }
 */
struct roseEtsiCallDeflection_ARG {
	/*! \brief Deflection address (Deflected-To address) */
	struct roseAddress deflection;

	/*! \brief TRUE if PresentationAllowedToDivertedToUser is present */
	u_int8_t presentation_allowed_to_diverted_to_user_present;

	/*! \brief TRUE if presentation is allowed (Optional) */
	u_int8_t presentation_allowed_to_diverted_to_user;
};


/*
 * ARGUMENT SEQUENCE {
 *     reroutingReason         DiversionReason,
 *     calledAddress           Address,
 *     reroutingCounter        DiversionCounter,
 *
 *     -- The User-user information element (optional),
 *     -- High layer compatibility information element (optional),
 *     -- Bearer capability information element
 *     -- and Low layer compatibility information element (optional)
 *     -- as specified in ETS 300 102-1 [11] subclause 4.5 shall be
 *     -- embedded in the q931InfoElement.
 *     q931InfoElement         Q931InformationElement,
 *     lastReroutingNr         [1] EXPLICIT PresentedNumberUnscreened,
 *     subscriptionOption      [2] EXPLICIT SubscriptionOption DEFAULT noNotification,
 *     callingPartySubaddress  [3] EXPLICIT PartySubaddress OPTIONAL
 * }
 */
struct roseEtsiCallRerouting_ARG {
	struct roseAddress called_address;

	/*! \brief The BC, HLC (optional), LLC (optional), and User-user (optional) information */
	struct roseQ931ie q931ie;
	/*! \brief q931ie.contents "allocated" after the stucture. */
	unsigned char q931ie_contents[ROSE_Q931_MAX_BC + ROSE_Q931_MAX_HLC +
		ROSE_Q931_MAX_LLC + ROSE_Q931_MAX_USER + 1];

	/*! \brief Last rerouting number */
	struct rosePresentedNumberUnscreened last_rerouting;

	/*! \brief Calling party subaddress (Optional) */
	struct rosePartySubaddress calling_subaddress;

	/*!
	 * \details
	 * unknown(0),
	 * cfu(1),
	 * cfb(2),
	 * cfnr(3),
	 * cdAlerting(4),
	 * cdImmediate(5)
	 */
	u_int8_t rerouting_reason;

	/*! \brief Range 1-5 */
	u_int8_t rerouting_counter;

	/*!
	 * \details
	 * DEFAULT noNotification
	 *
	 * \details
	 * noNotification(0),
	 * notificationWithoutDivertedToNr(1),
	 * notificationWithDivertedToNr(2)
	 */
	u_int8_t subscription_option;
};


/*
 * roseInterrogateServedUserNumbers_RES
 * ServedUserNumberList ::= SET SIZE (0..99) OF PartyNumber
 */
struct roseEtsiServedUserNumberList {
	/*!
	 * \brief SET SIZE (0..99) OF Served user numbers
	 * \note Reduced the size of the array to conserve
	 * potential stack usage.
	 */
	struct rosePartyNumber number[20];

	/*! \brief Number of Served user numbers present */
	u_int8_t num_records;
};


/*
 * ARGUMENT SEQUENCE {
 *     diversionReason     DiversionReason,
 *     subscriptionOption  SubscriptionOption,
 *     divertedToNumber    PresentedNumberUnscreened OPTIONAL
 * }
 */
struct roseEtsiDivertingLegInformation1_ARG {
	/*! \brief Diverted to number (Optional) */
	struct rosePresentedNumberUnscreened diverted_to;

	/*! \brief TRUE if DivertedTo is present */
	u_int8_t diverted_to_present;

	/*!
	 * \details
	 * unknown(0),
	 * cfu(1),
	 * cfb(2),
	 * cfnr(3),
	 * cdAlerting(4),
	 * cdImmediate(5)
	 */
	u_int8_t diversion_reason;

	/*!
	 * \details
	 * noNotification(0),
	 * notificationWithoutDivertedToNr(1),
	 * notificationWithDivertedToNr(2)
	 */
	u_int8_t subscription_option;
};

/*
 * ARGUMENT SEQUENCE {
 *     diversionCounter    DiversionCounter,
 *     diversionReason     DiversionReason,
 *     divertingNr         [1] EXPLICIT PresentedNumberUnscreened OPTIONAL,
 *     originalCalledNr    [2] EXPLICIT PresentedNumberUnscreened OPTIONAL
 * }
 */
struct roseEtsiDivertingLegInformation2_ARG {
	/*! \brief Diverting number (Optional) */
	struct rosePresentedNumberUnscreened diverting;

	/*! \brief Original called number (Optional) */
	struct rosePresentedNumberUnscreened original_called;

	/*! \brief TRUE if Diverting number is present */
	u_int8_t diverting_present;

	/*! \brief TRUE if OriginalCalled is present */
	u_int8_t original_called_present;

	/*!
	 * \details
	 * unknown(0),
	 * cfu(1),
	 * cfb(2),
	 * cfnr(3),
	 * cdAlerting(4),
	 * cdImmediate(5)
	 */
	u_int8_t diversion_reason;

	/*! \brief Range 1-5 */
	u_int8_t diversion_counter;
};

/*
 * ARGUMENT presentationAllowedIndicator PresentationAllowedIndicator
 */
struct roseEtsiDivertingLegInformation3_ARG {
	/*! \brief TRUE if presentation is allowed */
	u_int8_t presentation_allowed_indicator;
};


/* ------------------------------------------------------------------- */


/*
 * ARGUMENT LinkId
 */
struct roseEtsiExplicitEctExecute_ARG {
	int16_t link_id;
};

/*
 * ARGUMENT transferredToSubaddress PartySubaddress
 */
struct roseEtsiSubaddressTransfer_ARG {
	/*! \brief Transferred to subaddress */
	struct rosePartySubaddress subaddress;
};


/*
 * RESULT LinkId
 */
struct roseEtsiEctLinkIdRequest_RES {
	int16_t link_id;
};


/*
 * ARGUMENT SEQUENCE {
 *     ENUMERATED {
 *         alerting (0),
 *         active   (1)
 *     },
 *     redirectionNumber PresentedNumberUnscreened OPTIONAL
 * }
 */
struct roseEtsiEctInform_ARG {
	/*! \brief Redirection Number (Optional) */
	struct rosePresentedNumberUnscreened redirection;

	/*! \brief TRUE if the Redirection Number is present */
	u_int8_t redirection_present;

	/*! \details alerting(0), active(1) */
	u_int8_t status;
};


/*
 * ARGUMENT CallTransferIdentity
 */
struct roseEtsiEctLoopTest_ARG {
	int8_t call_transfer_id;
};

/*
 * RESULT LoopResult
 */
struct roseEtsiEctLoopTest_RES {
	/*!
	 * \details
	 * insufficientInformation(0),
	 * noLoopExists(1),
	 * simultaneousTransfer(2)
	 */
	u_int8_t loop_result;
};


/* ------------------------------------------------------------------- */


/*
 * ARGUMENT SEQUENCE {
 *     compatibilityMode CompatibilityMode,
 *
 *     -- The BC, HLC (optional) and LLC (optional) information
 *     -- elements shall be embedded in q931InfoElement
 *     q931InformationElement Q931InformationElement
 * }
 */
struct roseEtsiStatusRequest_ARG {
	/*! \brief The BC, HLC (optional) and LLC (optional) information */
	struct roseQ931ie q931ie;
	/*! \brief q931ie.contents "allocated" after the stucture. */
	unsigned char q931ie_contents[ROSE_Q931_MAX_BC + ROSE_Q931_MAX_HLC +
		ROSE_Q931_MAX_LLC + 1];

	/*! \details allBasicServices(0), oneOrMoreBasicServices(1) */
	u_int8_t compatibility_mode;
};

/*
 * RESULT StatusResult
 */
struct roseEtsiStatusRequest_RES {
	/*! \details compatibleAndFree(0), compatibleAndBusy(1), incompatible(2) */
	u_int8_t status;
};


/* ------------------------------------------------------------------- */


/*
 * CallLinkageID ::= INTEGER (0..127)
 * CCBSReference ::= INTEGER (0..127)
 */

/*
 * ARGUMENT callLinkageID CallLinkageID
 */
struct roseEtsiCallInfoRetain_ARG {
	/*! \brief Call Linkage Record ID */
	u_int8_t call_linkage_id;
};

/*
 * ARGUMENT callLinkageID CallLinkageID
 */
struct roseEtsiEraseCallLinkageID_ARG {
	/*! \brief Call Linkage Record ID */
	u_int8_t call_linkage_id;
};


/*
 * ARGUMENT callLinkageID CallLinkageID
 */
struct roseEtsiCCBSRequest_ARG {
	/*! \brief Call Linkage Record ID */
	u_int8_t call_linkage_id;
};

/*
 * RESULT SEQUENCE {
 *     recallMode          RecallMode,
 *     cCBSReference       CCBSReference
 * }
 */
struct roseEtsiCCBSRequest_RES {
	/*! \details globalRecall(0), specificRecall(1) */
	u_int8_t recall_mode;

	/*! \brief CCBS Record ID */
	u_int8_t ccbs_reference;
};


/*
 * ARGUMENT cCBSReference CCBSReference
 */
struct roseEtsiCCBSDeactivate_ARG {
	/*! \brief CCBS Record ID */
	u_int8_t ccbs_reference;
};


/*
 * ARGUMENT SEQUENCE {
 *     cCBSReference       CCBSReference OPTIONAL,
 *     partyNumberOfA      PartyNumber OPTIONAL
 * }
 */
struct roseEtsiCCBSInterrogate_ARG {
	/*! \brief Party A number (Optional) */
	struct rosePartyNumber a_party_number;

	/*! \brief TRUE if CCBSReference present */
	u_int8_t ccbs_reference_present;

	/*! \brief CCBS Record ID (optional) */
	u_int8_t ccbs_reference;
};

/*
 * -- The Bearer capability, High layer compatibility (optional)
 * -- and Low layer compatibility (optional) information elements
 * -- shall be embedded in q931InfoElement.
 * CallInformation ::= SEQUENCE {
 *     addressOfB          Address,
 *     q931InfoElement     Q931InformationElement,
 *     cCBSReference       CCBSReference,
 *     subAddressOfA       PartySubaddress OPTIONAL
 * }
 */
struct roseEtsiCallInformation {
	/*! \brief The BC, HLC (optional) and LLC (optional) information */
	struct roseQ931ie q931ie;
	/*! \brief q931ie.contents "allocated" after the stucture. */
	unsigned char q931ie_contents[ROSE_Q931_MAX_BC + ROSE_Q931_MAX_HLC +
		ROSE_Q931_MAX_LLC + 1];

	/*! \brief Address of B */
	struct roseAddress address_of_b;

	/*! \brief Subaddress of A (Optional) */
	struct rosePartySubaddress subaddress_of_a;

	/*! \brief CCBS Record ID */
	u_int8_t ccbs_reference;
};

/*
 * CallDetails ::= SEQUENCE SIZE(1..5) OF CallInformation
 */
struct roseEtsiCallDetailsList {
	struct roseEtsiCallInformation list[5];

	/*! \brief Number of CallDetails records present */
	u_int8_t num_records;
};

/*
 * RESULT SEQUENCE {
 *     recallMode          RecallMode,
 *     callDetails         CallDetails OPTIONAL
 * }
 */
struct roseEtsiCCBSInterrogate_RES {
	struct roseEtsiCallDetailsList call_details;

	/*! \details globalRecall(0), specificRecall(1) */
	u_int8_t recall_mode;
};


/*
 * ARGUMENT SEQUENCE {
 *     recallMode          RecallMode,
 *     cCBSReference       CCBSReference,
 *     addressOfB          Address,
 *     q931InfoElement     Q931InformationElement,
 *     eraseReason         CCBSEraseReason
 * }
 */
struct roseEtsiCCBSErase_ARG {
	/*! \brief The BC, HLC (optional) and LLC (optional) information */
	struct roseQ931ie q931ie;
	/*! \brief q931ie.contents "allocated" after the stucture. */
	unsigned char q931ie_contents[ROSE_Q931_MAX_BC + ROSE_Q931_MAX_HLC +
		ROSE_Q931_MAX_LLC + 1];

	/*! \brief Address of B */
	struct roseAddress address_of_b;

	/*! \details globalRecall(0), specificRecall(1) */
	u_int8_t recall_mode;

	/*! \brief CCBS Record ID */
	u_int8_t ccbs_reference;

	/*!
	 * \brief CCBS Erase reason
	 * \details
	 * normal-unspecified(0),
	 * t-CCBS2-timeout(1),
	 * t-CCBS3-timeout(2),
	 * basic-call-failed(3)
	 */
	u_int8_t reason;
};

/*
 * ARGUMENT SEQUENCE {
 *     recallMode          RecallMode,
 *     cCBSReference       CCBSReference,
 *     addressOfB          Address,
 *     q931InfoElement     Q931InformationElement
 * }
 */
struct roseEtsiCCBSRemoteUserFree_ARG {
	/*! \brief The BC, HLC (optional) and LLC (optional) information */
	struct roseQ931ie q931ie;
	/*! \brief q931ie.contents "allocated" after the stucture. */
	unsigned char q931ie_contents[ROSE_Q931_MAX_BC + ROSE_Q931_MAX_HLC +
		ROSE_Q931_MAX_LLC + 1];

	/*! \brief Address of B */
	struct roseAddress address_of_b;

	/*! \details globalRecall(0), specificRecall(1) */
	u_int8_t recall_mode;

	/*! \brief CCBS Record ID */
	u_int8_t ccbs_reference;
};

/*
 * ARGUMENT cCBSReference CCBSReference
 */
struct roseEtsiCCBSCall_ARG {
	/*! \brief CCBS Record ID */
	u_int8_t ccbs_reference;
};


/*
 * ARGUMENT SEQUENCE {
 *     recallMode          RecallMode,
 *     cCBSReference       CCBSReference,
 *     q931InfoElement     Q931InformationElement
 * }
 */
struct roseEtsiCCBSStatusRequest_ARG {
	/*! \brief The BC, HLC (optional) and LLC (optional) information */
	struct roseQ931ie q931ie;
	/*! \brief q931ie.contents "allocated" after the stucture. */
	unsigned char q931ie_contents[ROSE_Q931_MAX_BC + ROSE_Q931_MAX_HLC +
		ROSE_Q931_MAX_LLC + 1];

	/*! \details globalRecall(0), specificRecall(1) */
	u_int8_t recall_mode;

	/*! \brief CCBS Record ID */
	u_int8_t ccbs_reference;
};

/*
 * RESULT BOOLEAN -- free=TRUE, busy=FALSE
 */
struct roseEtsiCCBSStatusRequest_RES {
	/*! \brief TRUE if User A is free */
	u_int8_t free;
};


/*
 * ARGUMENT SEQUENCE {
 *     recallMode          RecallMode,
 *     cCBSReference       CCBSReference,
 *     addressOfB          Address,
 *     q931InfoElement     Q931InformationElement
 * }
 */
struct roseEtsiCCBSBFree_ARG {
	/*! \brief The BC, HLC (optional) and LLC (optional) information */
	struct roseQ931ie q931ie;
	/*! \brief q931ie.contents "allocated" after the stucture. */
	unsigned char q931ie_contents[ROSE_Q931_MAX_BC + ROSE_Q931_MAX_HLC +
		ROSE_Q931_MAX_LLC + 1];

	/*! \brief Address of B */
	struct roseAddress address_of_b;

	/*! \details globalRecall(0), specificRecall(1) */
	u_int8_t recall_mode;

	/*! \brief CCBS Record ID */
	u_int8_t ccbs_reference;
};

/*
 * ARGUMENT cCBSReference CCBSReference
 */
struct roseEtsiCCBSStopAlerting_ARG {
	/*! \brief CCBS Record ID */
	u_int8_t ccbs_reference;
};


/* ------------------------------------------------------------------- */


/*
 * ARGUMENT SEQUENCE {
 *     destinationAddress           Address,
 *
 *     -- contains HLC, LLC and BC information
 *     q931InfoElement              Q931InformationElement,
 *
 *     retentionSupported           [1] IMPLICIT BOOLEAN DEFAULT FALSE,
 *
 *     -- The use of this parameter is specified in
 *     -- EN 300 195-1 for interaction of CCBS with CLIP
 *     presentationAllowedIndicator [2] IMPLICIT BOOLEAN OPTIONAL,
 *
 *     -- The use of this parameter is specified in
 *     -- EN 300 195-1 for interaction of CCBS with CLIP
 *     originatingAddress           Address OPTIONAL
 * }
 */
struct roseEtsiCCBS_T_Request_ARG {
	/*! \brief The BC, HLC (optional) and LLC (optional) information */
	struct roseQ931ie q931ie;
	/*! \brief q931ie.contents "allocated" after the stucture. */
	unsigned char q931ie_contents[ROSE_Q931_MAX_BC + ROSE_Q931_MAX_HLC +
		ROSE_Q931_MAX_LLC + 1];

	/*! \brief Address of B */
	struct roseAddress destination;

	/*! \brief Caller-ID Address (Present if Originating.Party.LengthOfNumber is nonzero) */
	struct roseAddress originating;

	/*! \brief TRUE if the PresentationAllowedIndicator is present */
	u_int8_t presentation_allowed_indicator_present;

	/*! \brief TRUE if presentation is allowed for the originating address (optional) */
	u_int8_t presentation_allowed_indicator;

	/*! \brief TRUE if User A's CCBS request is continued if user B is busy again. */
	u_int8_t retention_supported;
};

/*
 * RESULT retentionSupported BOOLEAN   -- Default False
 */
struct roseEtsiCCBS_T_Request_RES {
	/*! \brief TRUE if User A's CCBS request is continued if user B is busy again. */
	u_int8_t retention_supported;
};


/* ------------------------------------------------------------------- */


/*
 * MessageID ::= SEQUENCE {
 *     messageRef  MessageRef,
 *     status      MessageStatus
 * }
 */
struct roseEtsiMessageID {
	/*! \brief Message reference number. (INTEGER (0..65535)) */
	u_int16_t reference_number;
	/*!
	 * \brief Message status
	 * \details
	 * added_message(0),
	 * removed_message(1)
	 */
	u_int8_t status;
};

/*
 * ARGUMENT SEQUENCE {
 *     receivingUserNr             PartyNumber,
 *     basicService                BasicService,
 *     controllingUserNr           [1] EXPLICIT PartyNumber     OPTIONAL,
 *     numberOfMessages            [2] EXPLICIT MessageCounter  OPTIONAL,
 *     controllingUserProvidedNr   [3] EXPLICIT PartyNumber     OPTIONAL,
 *     time                        [4] EXPLICIT GeneralizedTime OPTIONAL,
 *     messageId                   [5] EXPLICIT MessageID       OPTIONAL,
 *     mode                        [6] EXPLICIT InvocationMode  OPTIONAL
 * }
 */
struct roseEtsiMWIActivate_ARG {
	/*! \brief Number of messages in mailbox. (INTEGER (0..65535)) (Optional) */
	u_int16_t number_of_messages;

	/*! \brief Message ID (Status of this message) (Optional)*/
	struct roseEtsiMessageID message_id;

	/*! \brief Receiving user number (Who the message is for.) */
	struct rosePartyNumber receiving_user_number;
	/*! \brief Controlling user number (Mailbox number) (Optional) */
	struct rosePartyNumber controlling_user_number;
	/*! \brief Controlling user provided number (Caller-ID of party leaving message) (Optional) */
	struct rosePartyNumber controlling_user_provided_number;

	/*! \brief When message left. (optional) */
	struct roseGeneralizedTime time;

	/*!
	 * \brief Type of call leaving message.
	 * \details
	 * allServices(0),
	 * speech(1),
	 * unrestrictedDigitalInformation(2),
	 * audio3k1Hz(3),
	 * unrestrictedDigitalInformationWithTonesAndAnnouncements(4),
	 * multirate(5),
	 * telephony3k1Hz(32),
	 * teletex(33),
	 * telefaxGroup4Class1(34),
	 * videotexSyntaxBased(35),
	 * videotelephony(36),
	 * telefaxGroup2-3(37),
	 * telephony7kHz(38),
	 * euroFileTransfer(39),
	 * fileTransferAndAccessManagement(40),
	 * videoconference(41),
	 * audioGraphicConference(42)
	 */
	u_int8_t basic_service;
	/*!
	 * \brief Invocation mode (When it should be delivered.) (Optional)
	 * \details
	 * deferred(0),
	 * immediate(1),
	 * combined(2)
	 */
	u_int8_t mode;

	/*! \brief TRUE if NumberOfMessages present */
	u_int8_t number_of_messages_present;
	/*! \brief TRUE if time present */
	u_int8_t time_present;
	/*! \brief TRUE if MessageId present */
	u_int8_t message_id_present;
	/*! \brief TRUE if invocation mode present */
	u_int8_t mode_present;
};

/*
 * ARGUMENT SEQUENCE {
 *     receivingUserNr     PartyNumber,
 *     basicService        BasicService,
 *     controllingUserNr   PartyNumber    OPTIONAL,
 *     mode                InvocationMode OPTIONAL
 * }
 */
struct roseEtsiMWIDeactivate_ARG {
	/*! \brief Receiving user number (Who the message is for.) */
	struct rosePartyNumber receiving_user_number;
	/*! \brief Controlling user number (Mailbox number) (Optional) */
	struct rosePartyNumber controlling_user_number;

	/*!
	 * \brief Type of call leaving message.
	 * \details
	 * allServices(0),
	 * speech(1),
	 * unrestrictedDigitalInformation(2),
	 * audio3k1Hz(3),
	 * unrestrictedDigitalInformationWithTonesAndAnnouncements(4),
	 * multirate(5),
	 * telephony3k1Hz(32),
	 * teletex(33),
	 * telefaxGroup4Class1(34),
	 * videotexSyntaxBased(35),
	 * videotelephony(36),
	 * telefaxGroup2-3(37),
	 * telephony7kHz(38),
	 * euroFileTransfer(39),
	 * fileTransferAndAccessManagement(40),
	 * videoconference(41),
	 * audioGraphicConference(42)
	 */
	u_int8_t basic_service;

	/*!
	 * \brief Invocation mode (When it should be delivered.) (Optional)
	 * \details
	 * deferred(0),
	 * immediate(1),
	 * combined(2)
	 */
	u_int8_t mode;

	/*! \brief TRUE if invocation mode present */
	u_int8_t mode_present;
};

/*
 * ARGUMENT SEQUENCE {
 *     controllingUserNr           [1] EXPLICIT PartyNumber     OPTIONAL,
 *     basicService                [2] EXPLICIT BasicService    OPTIONAL,
 *     numberOfMessages            [3] EXPLICIT MessageCounter  OPTIONAL,
 *     controllingUserProvidedNr   [4] EXPLICIT PartyNumber     OPTIONAL,
 *     time                        [5] EXPLICIT GeneralizedTime OPTIONAL,
 *     messageId                   [6] EXPLICIT MessageID       OPTIONAL
 * }
 */
struct roseEtsiMWIIndicate_ARG {
	/*! \brief Number of messages in mailbox. (INTEGER (0..65535)) (Optional) */
	u_int16_t number_of_messages;

	/*! \brief Message ID (Status of this message) (Optional)*/
	struct roseEtsiMessageID message_id;

	/*! \brief Controlling user number (Mailbox number) (Optional) */
	struct rosePartyNumber controlling_user_number;
	/*! \brief Controlling user provided number (Caller-ID of party leaving message) (Optional) */
	struct rosePartyNumber controlling_user_provided_number;

	/*! \brief When message left. (optional) */
	struct roseGeneralizedTime time;

	/*!
	 * \brief Type of call leaving message.
	 * \details
	 * allServices(0),
	 * speech(1),
	 * unrestrictedDigitalInformation(2),
	 * audio3k1Hz(3),
	 * unrestrictedDigitalInformationWithTonesAndAnnouncements(4),
	 * multirate(5),
	 * telephony3k1Hz(32),
	 * teletex(33),
	 * telefaxGroup4Class1(34),
	 * videotexSyntaxBased(35),
	 * videotelephony(36),
	 * telefaxGroup2-3(37),
	 * telephony7kHz(38),
	 * euroFileTransfer(39),
	 * fileTransferAndAccessManagement(40),
	 * videoconference(41),
	 * audioGraphicConference(42)
	 */
	u_int8_t basic_service;

	/*! \brief TRUE if basic_service present */
	u_int8_t basic_service_present;
	/*! \brief TRUE if NumberOfMessages present */
	u_int8_t number_of_messages_present;
	/*! \brief TRUE if time present */
	u_int8_t time_present;
	/*! \brief TRUE if MessageId present */
	u_int8_t message_id_present;
};


/* ------------------------------------------------------------------- */


/*
 * Name ::= CHOICE {
 *     -- iso8859-1 is implied in namePresentationAllowedSimple.
 *     namePresentationAllowedSimple   [0] IMPLICIT NameData,
 *     namePresentationAllowedExtended [1] IMPLICIT NameSet,
 *
 *     -- iso8859-1 is implied in namePresentationRestrictedSimple.
 *     namePresentationRestrictedSimple    [2] IMPLICIT NameData,
 *     namePresentationRestrictedExtended  [3] IMPLICIT NameSet,
 *
 *     -- namePresentationRestrictedNull shall only be used in the
 *     -- case of interworking where the other network provides an
 *     -- indication that the name is restricted without the name itself.
 *     namePresentationRestrictedNull      [7] IMPLICIT NULL,
 *
 *     nameNotAvailable [4] IMPLICIT NULL
 * }
 *
 * NameSet ::= SEQUENCE {
 *     nameData        NameData,
 *
 *     -- If characterSet is not included, iso8859-1 is implied.
 *     characterSet    CharacterSet OPTIONAL -- DEFAULT iso8859-1
 * }
 *
 * -- The maximum allowed size of the name field is 50 octets.
 * -- The minimum required size of the name field is 1 octet.
 * NameData ::= OCTET STRING (SIZE (1..50))
 */
struct roseQsigName {
	/*!
	 * \details
	 * optional_name_not_present(0),
	 * presentation_allowed(1),
	 * presentation_restricted(2),
	 * presentation_restricted_null(3),
	 * name_not_available(4)
	 */
	u_int8_t presentation;

	/*!
	 * \details
	 * Set to iso8859-1 if not present in the encoding.
	 *
	 * \details
	 * unknown(0),
	 * iso8859-1(1),
	 * enum-value-withdrawn-by-ITU-T(2)
	 * iso8859-2(3),
	 * iso8859-3(4),
	 * iso8859-4(5),
	 * iso8859-5(6),
	 * iso8859-7(7),
	 * iso10646-BmpString(8),
	 * iso10646-utf-8String(9)
	 */
	u_int8_t char_set;

	/*! \brief Length of name data */
	u_int8_t length;

	/*! \brief Name string data */
	unsigned char data[50 + 1];
};

/*
 * NOTE: We are not going to record the Extension information
 * since it is manufacturer specific.
 *
 * ARGUMENT CHOICE {
 *     Name,
 *     SEQUENCE {
 *         Name,
 *         CHOICE {
 *             [5] IMPLICIT Extension,
 *             [6] IMPLICIT SEQUENCE OF Extension
 *         } OPTIONAL
 *     }
 * }
 */
struct roseQsigPartyName_ARG {
	struct roseQsigName name;
};


/* ------------------------------------------------------------------- */


/*
 * Time ::= SEQUENCE {
 *     lengthOfTimeUnit [1] IMPLICIT LengthOfTimeUnit,
 *     scale            [2] IMPLICIT Scale
 * }
 */
struct roseQsigAOCTime {
	/*! LengthOfTimeUnit ::= INTEGER (0..16777215) -- 24 bit number */
	u_int32_t length;
	/*!
	 * \details
	 * oneHundredthSecond(0),
	 * oneTenthSecond(1),
	 * oneSecond(2),
	 * tenSeconds(3),
	 * oneMinute(4),
	 * oneHour(5),
	 * twentyFourHours(6)
	 */
	u_int8_t scale;
};

/*
 * Amount ::= SEQUENCE {
 *     currencyAmount [1] IMPLICIT CurrencyAmount,
 *     multiplier     [2] IMPLICIT Multiplier
 * }
 */
struct roseQsigAOCAmount {
	/*! CurrencyAmount ::= INTEGER (0..16777215) -- 24 bit number */
	u_int32_t currency;
	/*!
	 * \details
	 * oneThousandth(0),
	 * oneHundredth(1),
	 * oneTenth(2),
	 * one(3),
	 * ten(4),
	 * hundred(5),
	 * thousand(6)
	 */
	u_int8_t multiplier;
};

/*
 * DurationCurrency ::= SEQUENCE {
 *     dCurrency       [1] IMPLICIT Currency,
 *     dAmount         [2] IMPLICIT Amount,
 *     dChargingType   [3] IMPLICIT ChargingType,
 *     dTime           [4] IMPLICIT Time,
 *     dGranularity    [5] IMPLICIT Time OPTIONAL
 * }
 */
struct roseQsigAOCDurationCurrency {
	struct roseQsigAOCAmount amount;
	struct roseQsigAOCTime time;
	/*! \brief dGranularity (optional) */
	struct roseQsigAOCTime granularity;
	/*!
	 * \brief Name of currency
	 * \details
	 * Currency ::= IA5String (SIZE (0..10))
	 * \note The empty string is the default currency for the network.
	 */
	unsigned char currency[10 + 1];
	/*!
	 * \details
	 * continuousCharging(0),
	 * stepFunction(1)
	 */
	u_int8_t charging_type;
	/*! TRUE if granularity time is present */
	u_int8_t granularity_present;
};

/*
 * FlatRateCurrency ::= SEQUENCE {
 *     fRCurrency      [1] IMPLICIT Currency,
 *     fRAmount        [2] IMPLICIT Amount
 * }
 */
struct roseQsigAOCFlatRateCurrency {
	struct roseQsigAOCAmount amount;
	/*!
	 * \brief Name of currency
	 * \details
	 * Currency ::= IA5String (SIZE (0..10))
	 * \note The empty string is the default currency for the network.
	 */
	unsigned char currency[10 + 1];
};

/*
 * VolumeRateCurrency ::= SEQUENCE {
 *     vRCurrency      [1] IMPLICIT Currency,
 *     vRAmount        [2] IMPLICIT Amount,
 *     vRVolumeUnit    [3] IMPLICIT VolumeUnit
 * }
 */
struct roseQsigAOCVolumeRateCurrency {
	struct roseQsigAOCAmount amount;
	/*!
	 * \brief Name of currency
	 * \details
	 * Currency ::= IA5String (SIZE (0..10))
	 * \note The empty string is the default currency for the network.
	 */
	unsigned char currency[10 + 1];
	/*!
	 * \brief Volume rate volume unit
	 * \details
	 * octet(0),
	 * segment(1),
	 * message(2)
	 */
	u_int8_t unit;
};

/*
 * AOCSCurrencyInfo ::= SEQUENCE {
 *     chargedItem                     ChargedItem,
 *     rateType CHOICE {
 *         durationCurrency            [1] IMPLICIT DurationCurrency,
 *         flatRateCurrency            [2] IMPLICIT FlatRateCurrency,
 *         volumeRateCurrency          [3] IMPLICIT VolumeRateCurrency,
 *         specialChargingCode         SpecialChargingCode,
 *         freeOfCharge                [4] IMPLICIT NULL,
 *         currencyInfoNotAvailable    [5] IMPLICIT NULL,
 *         freeOfChargefromBeginning   [6] IMPLICIT NULL
 *     }
 * }
 */
struct roseQsigAOCSCurrencyInfo {
	union {
		struct roseQsigAOCDurationCurrency duration;
		struct roseQsigAOCFlatRateCurrency flat_rate;
		struct roseQsigAOCVolumeRateCurrency volume_rate;
		/*! SpecialChargingCode ::= INTEGER (1..10) */
		u_int8_t special_charging_code;
	} u;
	/*!
	 * \brief Determine what is stored in the union.
	 * \details
	 * specialChargingCode(0),
	 * durationCurrency(1),
	 * flatRateCurrency(2),
	 * volumeRateCurrency(3),
	 * freeOfCharge(4),
	 * currencyInfoNotAvailable(5),
	 * freeOfChargeFromBeginning(6)
	 */
	u_int8_t currency_type;
	/*!
	 * \brief What service is being billed.
	 * \details
	 * basicCommunication(0),
	 * callAttempt(1),
	 * callSetup(2),
	 * userToUserInfo(3),
	 * operationOfSupplementaryServ(4)
	 */
	u_int8_t charged_item;
};

/*
 * AOCSCurrencyInfoList ::= SEQUENCE SIZE (1..10) OF AOCSCurrencyInfo
 */
struct roseQsigAOCSCurrencyInfoList {
	/*! \brief SEQUENCE SIZE (1..10) OF AOCSCurrencyInfo */
	struct roseQsigAOCSCurrencyInfo list[10];

	/*! \brief Number of AOCSCurrencyInfo records present */
	u_int8_t num_records;
};

/*
 * RecordedCurrency ::= SEQUENCE {
 *     rCurrency       [1] IMPLICIT Currency,
 *     rAmount         [2] IMPLICIT Amount
 * }
 */
struct roseQsigAOCRecordedCurrency {
	/*! Amount of currency involved. */
	struct roseQsigAOCAmount amount;
	/*!
	 * \brief Name of currency
	 * \details
	 * Currency ::= IA5String (SIZE (0..10))
	 * \note The empty string is the default currency for the network.
	 */
	unsigned char currency[10 + 1];
};

/*
 * ChargingAssociation ::= CHOICE {
 *     chargedNumber     [0] EXPLICIT PartyNumber,
 *     chargeIdentifier  ChargeIdentifier
 * }
 */
struct roseQsigAOCChargingAssociation {
	/*! chargeIdentifier: INTEGER (-32768..32767) -- 16 bit number */
	int16_t id;
	/*! chargedNumber */
	struct rosePartyNumber number;
	/*!
	 * \details
	 * charge_identifier(0)
	 * charged_number(1),
	 */
	u_int8_t type;
};

/*
 * AocRateArg ::= SEQUENCE {
 *     aocRate CHOICE
 *     {
 *         chargeNotAvailable      NULL,
 *         aocSCurrencyInfoList    AOCSCurrencyInfoList
 *     },
 *     rateArgExtension CHOICE {
 *         extension               [1] IMPLICIT Extension,
 *         multipleExtension       [2] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigAocRateArg_ARG {
	struct roseQsigAOCSCurrencyInfoList currency_info;
	/*!
	 * \details
	 * charge_not_available(0),
	 * currency_info_list(1)
	 */
	u_int8_t type;
};

/*
 * AocInterimArg ::= SEQUENCE {
 *     interimCharge CHOICE {
 *         chargeNotAvailable      [0] IMPLICIT NULL,
 *         freeOfCharge            [1] IMPLICIT NULL,
 *         specificCurrency        SEQUENCE {
 *             recordedCurrency    [1] IMPLICIT RecordedCurrency,
 *             interimBillingId    [2] IMPLICIT InterimBillingId OPTIONAL
 *         }
 *     },
 *     interimArgExtension CHOICE {
 *         extension           [1] IMPLICIT Extension,
 *         multipleExtension   [2] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigAocInterimArg_ARG {
	struct {
		/*! \brief recorded currency */
		struct roseQsigAOCRecordedCurrency recorded;
		/*!
		 * \brief InterimBillingId (optional)
		 * \details
		 * normalCharging(0),
		 * creditCardCharging(2)
		 */
		u_int8_t billing_id;
		/*! \brief TRUE if billing id is present */
		u_int8_t billing_id_present;
	} specific;
	/*!
	 * \details
	 * charge_not_available(0),
	 * free_of_charge(1),
	 * specific_currency(2)
	 */
	u_int8_t type;
};

/*
 * AocFinalArg ::= SEQUENCE {
 *     finalCharge CHOICE {
 *         chargeNotAvailable      [0] IMPLICIT NULL,
 *         freeOfCharge            [1] IMPLICIT NULL,
 *         specificCurrency        SEQUENCE {
 *             recordedCurrency    [1] IMPLICIT RecordedCurrency,
 *             finalBillingId      [2] IMPLICIT FinalBillingId OPTIONAL
 *         }
 *     },
 *     chargingAssociation ChargingAssociation OPTIONAL,
 *     finalArgExtension   CHOICE {
 *         extension           [1] IMPLICIT Extension,
 *         multipleExtension   [2] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigAocFinalArg_ARG {
	struct {
		/*! \brief recorded currency */
		struct roseQsigAOCRecordedCurrency recorded;
		/*!
		 * \brief FinalBillingId (optional)
		 * \details
		 * normalCharging(0),
		 * creditCardCharging(2),
		 * callForwardingUnconditional(3),
		 * callForwardingBusy(4),
		 * callForwardingNoReply(5),
		 * callDeflection(6),
		 * callTransfer(7)
		 */
		u_int8_t billing_id;
		/*! \brief TRUE if billing id is present */
		u_int8_t billing_id_present;
	} specific;

	/*! \brief chargingAssociation (optional) */
	struct roseQsigAOCChargingAssociation charging_association;

	/*! \brief TRUE if charging_association is present */
	u_int8_t charging_association_present;

	/*!
	 * \details
	 * charge_not_available(0),
	 * free_of_charge(1),
	 * specific_currency(2)
	 */
	u_int8_t type;
};

/*
 * ChargeRequestArg ::= SEQUENCE {
 *     adviceModeCombinations  SEQUENCE SIZE(0..7) OF AdviceModeCombination,
 *     chargeReqArgExtension   CHOICE {
 *         extension           [1] IMPLICIT Extension,
 *         multipleExtension   [2] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigChargeRequestArg_ARG {
	/*!
	 * \brief SEQUENCE SIZE(0..7) OF AdviceModeCombination
	 * \details
	 * rate(0) <charge rate provision>,
	 * rateInterim(1) <charge rate and interim charge provision>,
	 * rateFinal(2) <charge rate and final charge provision>,
	 * interim(3) <interim charge provision>,
	 * final(4) <final charge provision>,
	 * interimFinal(5) <interim charge and final charge provision>,
	 * rateInterimFinal(6) <charge rate, interim charge and final charge provision>
	 */
	u_int8_t advice_mode_combinations[7];

	/*! \brief Number of AdviceModeCombination values present */
	u_int8_t num_records;
};

/*
 * ChargeRequestRes ::= SEQUENCE {
 *     adviceModeCombination   AdviceModeCombination,
 *     chargeReqResExtension   CHOICE {
 *         extension           [1] IMPLICIT Extension,
 *         multipleExtension   [2] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigChargeRequestRes_RES {
	/*!
	 * \details
	 * rate(0) <charge rate provision>,
	 * rateInterim(1) <charge rate and interim charge provision>,
	 * rateFinal(2) <charge rate and final charge provision>,
	 * interim(3) <interim charge provision>,
	 * final(4) <final charge provision>,
	 * interimFinal(5) <interim charge and final charge provision>,
	 * rateInterimFinal(6) <charge rate, interim charge and final charge provision>
	 */
	u_int8_t advice_mode_combination;
};

/*
 * AocCompleteArg ::= SEQUENCE {
 *     chargedUser             PartyNumber,
 *     chargingAssociation     ChargingAssociation OPTIONAL,
 *     completeArgExtension    CHOICE {
 *         extension           [1] IMPLICIT Extension,
 *         multipleExtension   [2] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigAocCompleteArg_ARG {
	/*! \brief chargingAssociation (optional) */
	struct roseQsigAOCChargingAssociation charging_association;

	struct rosePartyNumber charged_user_number;

	/*! \brief TRUE if charging_association is present */
	u_int8_t charging_association_present;
};

/*
 * AocCompleteRes ::= SEQUENCE {
 *     chargingOption          ChargingOption,
 *     completeResExtension    CHOICE {
 *         extension           [1] IMPLICIT Extension,
 *         multipleExtension   [2] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigAocCompleteRes_RES {
	/*!
	 * \details
	 * aocFreeOfCharge(0),
	 * aocContinueCharging(1),
	 * aocStopCharging(2)
	 */
	u_int8_t charging_option;
};

/*
 * AocDivChargeReqArg ::= SEQUENCE {
 *     divertingUser           PartyNumber,
 *     chargingAssociation     ChargingAssociation OPTIONAL,
 *     diversionType           DiversionType,
 *     aocDivChargeReqArgExt   CHOICE {
 *         extension           [1] IMPLICIT Extension,
 *         multipleExtension   [2] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigAocDivChargeReqArg_ARG {
	/*! \brief chargingAssociation (optional) */
	struct roseQsigAOCChargingAssociation charging_association;

	struct rosePartyNumber diverting_user_number;

	/*! \brief TRUE if charging_association is present */
	u_int8_t charging_association_present;

	/*!
	 * \details
	 * callForwardingUnconditional(0),
	 * callForwardingBusy(1),
	 * callForwardingNoReply(2),
	 * callDeflection(3)
	 */
	u_int8_t diversion_type;
};


/* ------------------------------------------------------------------- */


/*
 * CTIdentifyRes ::= SEQUENCE {
 *     callIdentity        CallIdentity,
 *     reroutingNumber     PartyNumber,
 *     resultExtension     CHOICE {
 *         [6] IMPLICIT Extension,
 *         [7] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigCTIdentifyRes_RES {
	struct rosePartyNumber rerouting_number;

	/*! \brief CallIdentity ::= NumericString (SIZE (1..4)) */
	unsigned char call_id[4 + 1];
};

/*
 * CTInitiateArg ::= SEQUENCE {
 *     callIdentity        CallIdentity,
 *     reroutingNumber     PartyNumber,
 *     argumentExtension   CHOICE {
 *         [6] IMPLICIT Extension,
 *         [7] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigCTInitiateArg_ARG {
	struct rosePartyNumber rerouting_number;

	/*! \brief CallIdentity ::= NumericString (SIZE (1..4)) */
	unsigned char call_id[4 + 1];
};

/*
 * CTSetupArg ::= SEQUENCE {
 *     callIdentity        CallIdentity,
 *     argumentExtension   CHOICE {
 *         [0] IMPLICIT Extension,
 *         [1] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigCTSetupArg_ARG {
	/*! \brief CallIdentity ::= NumericString (SIZE (1..4)) */
	unsigned char call_id[4 + 1];
};

/*
 * CTActiveArg ::= SEQUENCE {
 *     connectedAddress        PresentedAddressScreened,
 *
 *     -- ISO/IEC 11572 information elements Party
 *     -- category and Progress indicator are conveyed
 *     basicCallInfoElements   PSS1InformationElement OPTIONAL,
 *     connectedName           Name OPTIONAL,
 *     argumentExtension       CHOICE {
 *         [9] IMPLICIT Extension,
 *         [10] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigCTActiveArg_ARG {
	/*! \brief connectedAddress */
	struct rosePresentedAddressScreened connected;

	/*! \brief basicCallInfoElements (optional) */
	struct roseQ931ie q931ie;
	/*! \brief q931ie.contents "allocated" after the stucture. */
	unsigned char q931ie_contents[ROSE_Q931_MAX_PROGRESS + 1];

	/*! \brief connectedName (optional) */
	struct roseQsigName connected_name;

	/*! \brief TRUE if connected_name is present */
	u_int8_t connected_name_present;
};

/*
 * CTCompleteArg ::= SEQUENCE {
 *     endDesignation          EndDesignation,
 *     redirectionNumber       PresentedNumberScreened,
 *
 *     -- ISO/IEC 11572 information elements Party
 *     -- category and Progress indicator are conveyed
 *     basicCallInfoElements   PSS1InformationElement OPTIONAL,
 *     redirectionName         Name OPTIONAL,
 *     callStatus              CallStatus DEFAULT answered,
 *     argumentExtension       CHOICE {
 *         [9] IMPLICIT Extension,
 *         [10] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigCTCompleteArg_ARG {
	/*! \brief redirectionNumber */
	struct rosePresentedNumberScreened redirection;

	/*! \brief basicCallInfoElements (optional) */
	struct roseQ931ie q931ie;
	/*! \brief q931ie.contents "allocated" after the stucture. */
	unsigned char q931ie_contents[ROSE_Q931_MAX_PROGRESS + 1];

	/*! \brief redirectionName (optional) */
	struct roseQsigName redirection_name;

	/*! \brief TRUE if redirection_name is present */
	u_int8_t redirection_name_present;

	/*!
	 * \brief endDesignation
	 * \details
	 * primaryEnd(0),
	 * secondaryEnd(1)
	 */
	u_int8_t end_designation;

	/*!
	 * \brief callStatus
	 * \details
	 * DEFAULT answered
	 *
	 * \details
	 * answered(0),
	 * alerting(1)
	 */
	u_int8_t call_status;
};

/*
 * CTUpdateArg ::= SEQUENCE {
 *     redirectionNumber       PresentedNumberScreened,
 *     redirectionName         Name OPTIONAL,
 *
 *     -- ISO/IEC 11572 information elements Party
 *     -- category and Progress indicator are conveyed
 *     basicCallInfoElements   PSS1InformationElement OPTIONAL,
 *     argumentExtension       CHOICE {
 *         [9] IMPLICIT Extension,
 *         [10] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigCTUpdateArg_ARG {
	/*! \brief redirectionNumber */
	struct rosePresentedNumberScreened redirection;

	/*! \brief basicCallInfoElements (optional) */
	struct roseQ931ie q931ie;
	/*! \brief q931ie.contents "allocated" after the stucture. */
	unsigned char q931ie_contents[ROSE_Q931_MAX_PROGRESS + 1];

	/*! \brief redirectionName (optional) */
	struct roseQsigName redirection_name;

	/*! \brief TRUE if redirection_name is present */
	u_int8_t redirection_name_present;
};

/*
 * SubaddressTransferArg ::= SEQUENCE {
 *     redirectionSubaddress   PartySubaddress,
 *     argumentExtension       CHOICE {
 *         [0] IMPLICIT Extension,
 *         [1] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigSubaddressTransferArg_ARG {
	struct rosePartySubaddress redirection_subaddress;
};


/* ------------------------------------------------------------------- */


/*
 * IntResult ::= SEQUENCE {
 *     servedUserNr        PartyNumber,
 *     basicService        BasicService,
 *     procedure           Procedure,
 *     divertedToAddress   Address,
 *     remoteEnabled       BOOLEAN DEFAULT FALSE,
 *     extension           CHOICE {
 *         [1] IMPLICIT Extension,
 *         [2] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigForwardingRecord {
	/*! \brief Diverted to address */
	struct roseAddress diverted_to;

	struct rosePartyNumber served_user_number;

	/*!
	 * \details
	 * allServices(0),
	 * speech(1),
	 * unrestrictedDigitalInformation(2),
	 * audio3100Hz(3),
	 * telephony(32),
	 * teletex(33),
	 * telefaxGroup4Class1(34),
	 * videotexSyntaxBased(35),
	 * videotelephony(36)
	 */
	u_int8_t basic_service;

	/*! \details cfu(0), cfb(1), cfnr(2) */
	u_int8_t procedure;

	/*! \brief remoteEnabled BOOLEAN DEFAULT FALSE */
	u_int8_t remote_enabled;
};

/*
 * roseQsigInterrogateDiversionQ_REQ
 * IntResultList ::= SET SIZE (0..29) OF IntResult
 */
struct roseQsigForwardingList {
	/*!
	 * \brief SET SIZE (0..29) OF Forwarding Records
	 * \note Reduced the size of the array to conserve
	 * potential stack usage.
	 */
	struct roseQsigForwardingRecord list[10];

	/*! \brief Number of Forwarding records present */
	u_int8_t num_records;
};

/*
 * ARGUMENT SEQUENCE {
 *     procedure           Procedure,
 *     basicService        BasicService,
 *     divertedToAddress   Address,
 *     servedUserNr        PartyNumber,
 *     activatingUserNr    PartyNumber,
 *     extension           CHOICE {
 *         [1] IMPLICIT Extension,
 *         [2] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigActivateDiversionQ_ARG {
	/*! \brief divertedToAddress */
	struct roseAddress diverted_to;

	struct rosePartyNumber served_user_number;
	struct rosePartyNumber activating_user_number;

	/*! \details cfu(0), cfb(1), cfnr(2) */
	u_int8_t procedure;

	/*!
	 * \details
	 * allServices(0),
	 * speech(1),
	 * unrestrictedDigitalInformation(2),
	 * audio3100Hz(3),
	 * telephony(32),
	 * teletex(33),
	 * telefaxGroup4Class1(34),
	 * videotexSyntaxBased(35),
	 * videotelephony(36)
	 */
	u_int8_t basic_service;
};

/*
 * ARGUMENT SEQUENCE {
 *     procedure           Procedure,
 *     basicService        BasicService,
 *     servedUserNr        PartyNumber,
 *     deactivatingUserNr  PartyNumber,
 *     extension           CHOICE {
 *         [1] IMPLICIT Extension,
 *         [2] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigDeactivateDiversionQ_ARG {
	struct rosePartyNumber served_user_number;
	struct rosePartyNumber deactivating_user_number;

	/*! \details cfu(0), cfb(1), cfnr(2) */
	u_int8_t procedure;

	/*!
	 * \details
	 * allServices(0),
	 * speech(1),
	 * unrestrictedDigitalInformation(2),
	 * audio3100Hz(3),
	 * telephony(32),
	 * teletex(33),
	 * telefaxGroup4Class1(34),
	 * videotexSyntaxBased(35),
	 * videotelephony(36)
	 */
	u_int8_t basic_service;
};

/*
 * ARGUMENT SEQUENCE {
 *     procedure           Procedure,
 *     basicService        BasicService DEFAULT allServices,
 *     servedUserNr        PartyNumber,
 *     interrogatingUserNr PartyNumber,
 *     extension           CHOICE {
 *         [1] IMPLICIT Extension,
 *         [2] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigInterrogateDiversionQ_ARG {
	struct rosePartyNumber served_user_number;
	struct rosePartyNumber interrogating_user_number;

	/*! \details cfu(0), cfb(1), cfnr(2) */
	u_int8_t procedure;

	/*!
	 * \details
	 * DEFAULT allServices
	 *
	 * \details
	 * allServices(0),
	 * speech(1),
	 * unrestrictedDigitalInformation(2),
	 * audio3100Hz(3),
	 * telephony(32),
	 * teletex(33),
	 * telefaxGroup4Class1(34),
	 * videotexSyntaxBased(35),
	 * videotelephony(36)
	 */
	u_int8_t basic_service;
};

/*
 * ARGUMENT SEQUENCE {
 *     servedUserNr        PartyNumber,
 *     basicService        BasicService,
 *     divertedToNr        PartyNumber,
 *     extension           CHOICE {
 *         [1] IMPLICIT Extension,
 *         [2] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigCheckRestriction_ARG {
	struct rosePartyNumber served_user_number;
	struct rosePartyNumber diverted_to_number;

	/*!
	 * \details
	 * allServices(0),
	 * speech(1),
	 * unrestrictedDigitalInformation(2),
	 * audio3100Hz(3),
	 * telephony(32),
	 * teletex(33),
	 * telefaxGroup4Class1(34),
	 * videotexSyntaxBased(35),
	 * videotelephony(36)
	 */
	u_int8_t basic_service;
};

/*
 * ARGUMENT SEQUENCE {
 *     reroutingReason             DiversionReason,
 *     originalReroutingReason     [0] IMPLICIT DiversionReason OPTIONAL,
 *     calledAddress               Address,
 *     diversionCounter            INTEGER (1..15),
 *
 *     -- The basic call information elements Bearer capability,
 *     -- High layer compatibility, Low layer compatibity
 *     -- and Progress indicator can be embedded in the
 *     -- pSS1InfoElement in accordance with 6.5.3.1.5.
 *     pSS1InfoElement             PSS1InformationElement,
 *     lastReroutingNr             [1] EXPLICIT PresentedNumberUnscreened,
 *     subscriptionOption          [2] IMPLICIT SubscriptionOption,
 *     callingPartySubaddress      [3] EXPLICIT PartySubaddress OPTIONAL,
 *     callingNumber               [4] EXPLICIT PresentedNumberScreened,
 *     callingName                 [5] EXPLICIT Name OPTIONAL,
 *     originalCalledNr            [6] EXPLICIT PresentedNumberUnscreened OPTIONAL,
 *     redirectingName             [7] EXPLICIT Name OPTIONAL,
 *     originalCalledName          [8] EXPLICIT Name OPTIONAL,
 *     extension                   CHOICE {
 *         [9] IMPLICIT Extension,
 *         [10] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigCallRerouting_ARG {
	/*! \brief calledAddress */
	struct roseAddress called;

	/*! \brief lastReroutingNr */
	struct rosePresentedNumberUnscreened last_rerouting;

	/*! \brief originalCalledNr (optional) */
	struct rosePresentedNumberUnscreened original_called;

	/*!
	 * \brief callingPartySubaddress (optional)
	 * The subaddress is present if the length is nonzero.
	 */
	struct rosePartySubaddress calling_subaddress;

	/*! \brief callingNumber */
	struct rosePresentedNumberScreened calling;

	/*! \brief callingName (optional) */
	struct roseQsigName calling_name;

	/*! \brief redirectingName (optional) */
	struct roseQsigName redirecting_name;

	/*! \brief originalCalledName (optional) */
	struct roseQsigName original_called_name;

	/*!
	 * \brief The BC, HLC (optional), LLC (optional),
	 * and progress indicator(s) information.
	 */
	struct roseQ931ie q931ie;
	/*! \brief q931ie.contents "allocated" after the stucture. */
	unsigned char q931ie_contents[ROSE_Q931_MAX_BC + ROSE_Q931_MAX_HLC +
		ROSE_Q931_MAX_LLC + ROSE_Q931_MAX_PROGRESS + 1];

	/*! \brief TRUE if calling_name is present */
	u_int8_t calling_name_present;

	/*! \brief TRUE if redirecting_name is present */
	u_int8_t redirecting_name_present;

	/*! \brief TRUE if original_called_name is present */
	u_int8_t original_called_name_present;

	/*! \brief TRUE if original_called number is present */
	u_int8_t original_called_present;

	/*!
	 * \details
	 * unknown(0),
	 * cfu(1),
	 * cfb(2),
	 * cfnr(3)
	 *
	 * \note The value unknown is only used if received from
	 * another network when interworking.
	 */
	u_int8_t rerouting_reason;

	/*!
	 * \brief originalReroutingReason (optional)
	 *
	 * \details
	 * unknown(0),
	 * cfu(1),
	 * cfb(2),
	 * cfnr(3)
	 *
	 * \note The value unknown is only used if received from
	 * another network when interworking.
	 */
	u_int8_t original_rerouting_reason;

	/*! \brief TRUE if original_rerouting_reason is present */
	u_int8_t original_rerouting_reason_present;

	/*! \brief diversionCounter INTEGER (1..15) */
	u_int8_t diversion_counter;

	/*!
	 * \brief subscriptionOption
	 *
	 * \details
	 * noNotification(0),
	 * notificationWithoutDivertedToNr(1),
	 * notificationWithDivertedToNr(2)
	 */
	u_int8_t subscription_option;
};

/*
 * ARGUMENT SEQUENCE {
 *     diversionReason     DiversionReason,
 *     subscriptionOption  SubscriptionOption,
 *     nominatedNr         PartyNumber,
 *     extension           CHOICE {
 *         [9] IMPLICIT Extension,
 *         [10] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigDivertingLegInformation1_ARG {
	struct rosePartyNumber nominated_number;

	/*!
	 * \details
	 * unknown(0),
	 * cfu(1),
	 * cfb(2),
	 * cfnr(3)
	 *
	 * \note The value unknown is only used if received from
	 * another network when interworking.
	 */
	u_int8_t diversion_reason;

	/*!
	 * \brief subscriptionOption
	 *
	 * \details
	 * noNotification(0),
	 * notificationWithoutDivertedToNr(1),
	 * notificationWithDivertedToNr(2)
	 */
	u_int8_t subscription_option;
};

/*
 * ARGUMENT SEQUENCE {
 *     diversionCounter             INTEGER (1..15),
 *     diversionReason              DiversionReason,
 *     originalDiversionReason      [0] IMPLICIT DiversionReason OPTIONAL,
 *
 *     -- The divertingNr element is mandatory except in the case
 *     -- of interworking.
 *     divertingNr                  [1] EXPLICIT PresentedNumberUnscreened OPTIONAL,
 *     originalCalledNr             [2] EXPLICIT PresentedNumberUnscreened OPTIONAL,
 *     redirectingName              [3] EXPLICIT Name OPTIONAL,
 *     originalCalledName           [4] EXPLICIT Name OPTIONAL,
 *     extension                    CHOICE {
 *         [5] IMPLICIT Extension,
 *         [6] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigDivertingLegInformation2_ARG {
	/*! \brief divertingNr (optional) */
	struct rosePresentedNumberUnscreened diverting;

	/*! \brief originalCalledNr (optional) */
	struct rosePresentedNumberUnscreened original_called;

	/*! \brief redirectingName (optional) */
	struct roseQsigName redirecting_name;

	/*! \brief originalCalledName (optional) */
	struct roseQsigName original_called_name;

	/*! \brief diversionCounter INTEGER (1..15) */
	u_int8_t diversion_counter;

	/*!
	 * \details
	 * unknown(0),
	 * cfu(1),
	 * cfb(2),
	 * cfnr(3)
	 *
	 * \note The value unknown is only used if received from
	 * another network when interworking.
	 */
	u_int8_t diversion_reason;

	/*!
	 * \brief originalDiversionReason (optional)
	 *
	 * \details
	 * unknown(0),
	 * cfu(1),
	 * cfb(2),
	 * cfnr(3)
	 *
	 * \note The value unknown is only used if received from
	 * another network when interworking.
	 */
	u_int8_t original_diversion_reason;

	/*! \brief TRUE if original_diversion_reason is present */
	u_int8_t original_diversion_reason_present;

	/*! \brief TRUE if diverting number is present */
	u_int8_t diverting_present;

	/*! \brief TRUE if original_called number is present */
	u_int8_t original_called_present;

	/*! \brief TRUE if redirecting_name is present */
	u_int8_t redirecting_name_present;

	/*! \brief TRUE if original_called_name is present */
	u_int8_t original_called_name_present;
};

/*
 * ARGUMENT SEQUENCE {
 *     presentationAllowedIndicator    PresentationAllowedIndicator, -- BOOLEAN
 *     redirectionName                 [0] EXPLICIT Name OPTIONAL,
 *     extension                       CHOICE {
 *         [1] IMPLICIT Extension,
 *         [2] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigDivertingLegInformation3_ARG {
	/*! \brief redirectionName (optional) */
	struct roseQsigName redirection_name;

	/*! \brief TRUE if redirection_name is present */
	u_int8_t redirection_name_present;

	/*! \brief TRUE if presentation is allowed */
	u_int8_t presentation_allowed_indicator;
};


/* ------------------------------------------------------------------- */


/*
 * CcExtension ::= CHOICE {
 *     none        NULL,
 *     single      [14] IMPLICIT Extension,
 *     multiple    [15] IMPLICIT SEQUENCE OF Extension
 * }
 */

/*
 * CcRequestArg ::= SEQUENCE {
 *     numberA                 PresentedNumberUnscreened,
 *     numberB                 PartyNumber,
 *
 *     -- permitted information elements are:
 *     -- Bearer capability;
 *     -- Low layer compatibility;
 *     -- High layer compatibility
 *     service                 PSS1InformationElement,
 *     subaddrA                [10] EXPLICIT PartySubaddress OPTIONAL,
 *     subaddrB                [11] EXPLICIT PartySubaddress OPTIONAL,
 *     can-retain-service      [12] IMPLICIT BOOLEAN DEFAULT FALSE,
 *
 *     -- TRUE: signalling connection to be retained;
 *     -- FALSE: signalling connection to be released;
 *     -- omission: release or retain signalling connection
 *     retain-sig-connection   [13] IMPLICIT BOOLEAN OPTIONAL,
 *     extension               CcExtension OPTIONAL
 * }
 */
struct roseQsigCcRequestArg {
	struct rosePresentedNumberUnscreened number_a;
	struct rosePartyNumber number_b;

	/*!
	 * \brief subaddrA (optional)
	 * The subaddress is present if the length is nonzero.
	 */
	struct rosePartySubaddress subaddr_a;

	/*!
	 * \brief subaddrB (optional)
	 * The subaddress is present if the length is nonzero.
	 */
	struct rosePartySubaddress subaddr_b;

	/*!
	 * \brief The BC, HLC (optional) and LLC (optional) information.
	 * \note The ASN.1 field name is service.
	 */
	struct roseQ931ie q931ie;
	/*! \brief q931ie.contents "allocated" after the stucture. */
	unsigned char q931ie_contents[ROSE_Q931_MAX_BC + ROSE_Q931_MAX_HLC +
		ROSE_Q931_MAX_LLC + 1];

	/*! \brief TRUE if can retain service (DEFAULT FALSE) */
	u_int8_t can_retain_service;

	/*!
	 * \brief TRUE if retain_sig_connection present
	 * \note If not present then the signaling connection could be
	 * released or retained.
	 */
	u_int8_t retain_sig_connection_present;

	/*!
	 * \brief Determine if the signalling connection should be retained.
	 * \note TRUE if signalling connection to be retained.
	 * \note FALSE if signalling connection to be released.
	 */
	u_int8_t retain_sig_connection;
};

/*
 * CcRequestRes ::= SEQUENCE {
 *     no-path-reservation     [0] IMPLICIT BOOLEAN DEFAULT FALSE,
 *     retain-service          [1] IMPLICIT BOOLEAN DEFAULT FALSE,
 *     extension               CcExtension OPTIONAL
 * }
 */
struct roseQsigCcRequestRes {
	/*! \brief TRUE if no path reservation. (DEFAULT FALSE) */
	u_int8_t no_path_reservation;

	/*! \brief TRUE if agree to retain service (DEFAULT FALSE) */
	u_int8_t retain_service;
};

/*
 * CcOptionalArg ::= CHOICE {
 *     fullArg                 [0] IMPLICIT SEQUENCE {
 *         numberA             PartyNumber,
 *         numberB             PartyNumber,
 *
 *         -- permitted information elements are:
 *         -- Bearer capability;
 *         -- Low layer compatibility;
 *         -- High layer compatibility.
 *         service             PSS1InformationElement,
 *         subaddrA            [10] EXPLICIT PartySubaddress OPTIONAL,
 *         subaddrB            [11] EXPLICIT PartySubaddress OPTIONAL,
 *         extension           CcExtension OPTIONAL
 *     },
 *     extArg                  CcExtension
 * }
 */
struct roseQsigCcOptionalArg {
#if 1	/* The conditional is here to indicate fullArg values grouping. */
	struct rosePartyNumber number_a;
	struct rosePartyNumber number_b;

	/*!
	 * \brief subaddrA (optional)
	 * The subaddress is present if the length is nonzero.
	 */
	struct rosePartySubaddress subaddr_a;

	/*!
	 * \brief subaddrB (optional)
	 * The subaddress is present if the length is nonzero.
	 */
	struct rosePartySubaddress subaddr_b;

	/*!
	 * \brief The BC, HLC (optional) and LLC (optional) information.
	 * \note The ASN.1 field name is service.
	 */
	struct roseQ931ie q931ie;
	/*! \brief q931ie.contents "allocated" after the stucture. */
	unsigned char q931ie_contents[ROSE_Q931_MAX_BC + ROSE_Q931_MAX_HLC +
		ROSE_Q931_MAX_LLC + 1];
#endif	/* end fullArg values */

	/*! \brief TRUE if the fullArg values are present. */
	u_int8_t full_arg_present;
};


/* ------------------------------------------------------------------- */


/*
 * MsgCentreId ::= CHOICE {
 *     integer         [0] IMPLICIT INTEGER (0..65535),
 *
 *     -- The party number must be a complete number as required
 *     -- for routing purposes.
 *     partyNumber     [1] EXPLICIT PartyNumber,
 *     numericString   [2] IMPLICIT NumericString (SIZE (1..10))
 * }
 */
struct roseQsigMsgCentreId {
	union {
		/*! \brief INTEGER (0..65535) */
		u_int16_t integer;

		/*!
		 * \note The party number must be a complete number as required
		 * for routing purposes.
		 */
		struct rosePartyNumber number;

		/*! \brief NumericString (SIZE (1..10)) */
		unsigned char str[10 + 1];
	} u;

	/*!
	 * \details
	 * integer(0),
	 * partyNumber(1),
	 * numericString(2)
	 */
	u_int8_t type;
};

/*
 * MWIActivateArg ::= SEQUENCE {
 *     servedUserNr            PartyNumber,
 *     basicService            BasicService,
 *     msgCentreId             MsgCentreId OPTIONAL,
 *     nbOfMessages            [3] IMPLICIT NbOfMessages OPTIONAL,
 *     originatingNr           [4] EXPLICIT PartyNumber OPTIONAL,
 *     timestamp               TimeStamp OPTIONAL,
 *
 *     -- The value 0 means the highest priority and 9 the lowest
 *     priority                [5] IMPLICIT INTEGER (0..9) OPTIONAL,
 *     argumentExt             CHOICE {
 *         extension           [6] IMPLICIT Extension,
 *         multipleExtension   [7] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigMWIActivateArg {
	/*! \brief NbOfMessages ::= INTEGER (0..65535) (optional) */
	u_int16_t number_of_messages;

	/*! \brief msgCentreId (optional) */
	struct roseQsigMsgCentreId msg_centre_id;

	struct rosePartyNumber served_user_number;

	/*! \brief originatingNr (optional) (Number present if length is nonzero) */
	struct rosePartyNumber originating_number;

	/*! \brief GeneralizedTime (SIZE (12..19)) (optional) */
	struct roseGeneralizedTime timestamp;

	/*!
	 * \details
	 * allServices(0),
	 * speech(1),
	 * unrestrictedDigitalInformation(2),
	 * audio3100Hz(3),
	 * telephony(32),
	 * teletex(33),
	 * telefaxGroup4Class1(34),
	 * videotextSyntaxBased(35),
	 * videotelephony(36),
	 * telefaxGroup2-3(37),
	 * reservedNotUsed1(38),
	 * reservedNotUsed2(39),
	 * reservedNotUsed3(40),
	 * reservedNotUsed4(41),
	 * reservedNotUsed5(42),
	 * email(51),
	 * video(52),
	 * fileTransfer(53),
	 * shortMessageService(54),
	 * speechAndVideo(55),
	 * speechAndFax(56),
	 * speechAndEmail(57),
	 * videoAndFax(58),
	 * videoAndEmail(59),
	 * faxAndEmail(60),
	 * speechVideoAndFax(61),
	 * speechVideoAndEmail(62),
	 * speechFaxAndEmail(63),
	 * videoFaxAndEmail(64),
	 * speechVideoFaxAndEmail(65),
	 * multimediaUnknown(66),
	 * serviceUnknown(67),
	 * futureReserve1(68),
	 * futureReserve2(69),
	 * futureReserve3(70),
	 * futureReserve4(71),
	 * futureReserve5(72),
	 * futureReserve6(73),
	 * futureReserve7(74),
	 * futureReserve8(75)
	 */
	u_int8_t basic_service;

	/*!
	 * \brief INTEGER (0..9) (optional)
	 * \note The value 0 means the highest priority and 9 the lowest.
	 */
	u_int8_t priority;

	/*! \brief TRUE if msg_centre_id is present */
	u_int8_t msg_centre_id_present;

	/*! \brief TRUE if number_of_messages is present */
	u_int8_t number_of_messages_present;

	/*! \brief TRUE if timestamp is present */
	u_int8_t timestamp_present;

	/*! \brief TRUE if priority is present */
	u_int8_t priority_present;
};

/*
 * MWIDeactivateArg ::= SEQUENCE {
 *     servedUserNr            PartyNumber,
 *     basicService            BasicService,
 *     msgCentreId             MsgCentreId OPTIONAL,
 *     argumentExt             CHOICE {
 *         extension           [3] IMPLICIT Extension,
 *         multipleExtension   [4] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigMWIDeactivateArg {
	/*! \brief msgCentreId (optional) */
	struct roseQsigMsgCentreId msg_centre_id;

	struct rosePartyNumber served_user_number;

	/*!
	 * \details
	 * allServices(0),
	 * speech(1),
	 * unrestrictedDigitalInformation(2),
	 * audio3100Hz(3),
	 * telephony(32),
	 * teletex(33),
	 * telefaxGroup4Class1(34),
	 * videotextSyntaxBased(35),
	 * videotelephony(36),
	 * telefaxGroup2-3(37),
	 * reservedNotUsed1(38),
	 * reservedNotUsed2(39),
	 * reservedNotUsed3(40),
	 * reservedNotUsed4(41),
	 * reservedNotUsed5(42),
	 * email(51),
	 * video(52),
	 * fileTransfer(53),
	 * shortMessageService(54),
	 * speechAndVideo(55),
	 * speechAndFax(56),
	 * speechAndEmail(57),
	 * videoAndFax(58),
	 * videoAndEmail(59),
	 * faxAndEmail(60),
	 * speechVideoAndFax(61),
	 * speechVideoAndEmail(62),
	 * speechFaxAndEmail(63),
	 * videoFaxAndEmail(64),
	 * speechVideoFaxAndEmail(65),
	 * multimediaUnknown(66),
	 * serviceUnknown(67),
	 * futureReserve1(68),
	 * futureReserve2(69),
	 * futureReserve3(70),
	 * futureReserve4(71),
	 * futureReserve5(72),
	 * futureReserve6(73),
	 * futureReserve7(74),
	 * futureReserve8(75)
	 */
	u_int8_t basic_service;

	/*! \brief TRUE if msg_centre_id is present */
	u_int8_t msg_centre_id_present;
};

/*
 * MWIInterrogateArg ::= SEQUENCE {
 *     servedUserNr            PartyNumber,
 *     basicService            BasicService,
 *     msgCentreId             MsgCentreId OPTIONAL,
 *     argumentExt             CHOICE {
 *         extension           [3] IMPLICIT Extension,
 *         multipleExtension   [4] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigMWIInterrogateArg {
	/*! \brief msgCentreId (optional) */
	struct roseQsigMsgCentreId msg_centre_id;

	struct rosePartyNumber served_user_number;

	/*!
	 * \details
	 * allServices(0),
	 * speech(1),
	 * unrestrictedDigitalInformation(2),
	 * audio3100Hz(3),
	 * telephony(32),
	 * teletex(33),
	 * telefaxGroup4Class1(34),
	 * videotextSyntaxBased(35),
	 * videotelephony(36),
	 * telefaxGroup2-3(37),
	 * reservedNotUsed1(38),
	 * reservedNotUsed2(39),
	 * reservedNotUsed3(40),
	 * reservedNotUsed4(41),
	 * reservedNotUsed5(42),
	 * email(51),
	 * video(52),
	 * fileTransfer(53),
	 * shortMessageService(54),
	 * speechAndVideo(55),
	 * speechAndFax(56),
	 * speechAndEmail(57),
	 * videoAndFax(58),
	 * videoAndEmail(59),
	 * faxAndEmail(60),
	 * speechVideoAndFax(61),
	 * speechVideoAndEmail(62),
	 * speechFaxAndEmail(63),
	 * videoFaxAndEmail(64),
	 * speechVideoFaxAndEmail(65),
	 * multimediaUnknown(66),
	 * serviceUnknown(67),
	 * futureReserve1(68),
	 * futureReserve2(69),
	 * futureReserve3(70),
	 * futureReserve4(71),
	 * futureReserve5(72),
	 * futureReserve6(73),
	 * futureReserve7(74),
	 * futureReserve8(75)
	 */
	u_int8_t basic_service;

	/*! \brief TRUE if msg_centre_id is present */
	u_int8_t msg_centre_id_present;
};

/*
 * MWIInterrogateResElt ::= SEQUENCE {
 *     basicService            BasicService,
 *     msgCentreId             MsgCentreId OPTIONAL,
 *     nbOfMessages            [3] IMPLICIT NbOfMessages OPTIONAL,
 *     originatingNr           [4] EXPLICIT PartyNumber OPTIONAL,
 *     timestamp               TimeStamp OPTIONAL,
 *
 *     -- The value 0 means the highest priority and 9 the lowest
 *     priority                [5] IMPLICIT INTEGER (0..9) OPTIONAL,
 *     argumentExt             CHOICE {
 *         extension           [6] IMPLICIT Extension,
 *         multipleExtension   [7] IMPLICIT SEQUENCE OF Extension
 *     } OPTIONAL
 * }
 */
struct roseQsigMWIInterrogateResElt {
	/*! \brief NbOfMessages ::= INTEGER (0..65535) (optional) */
	u_int16_t number_of_messages;

	/*! \brief msgCentreId (optional) */
	struct roseQsigMsgCentreId msg_centre_id;

	/*! \brief originatingNr (optional) (Number present if length is nonzero) */
	struct rosePartyNumber originating_number;

	/*! \brief GeneralizedTime (SIZE (12..19)) (optional) */
	struct roseGeneralizedTime timestamp;

	/*!
	 * \details
	 * allServices(0),
	 * speech(1),
	 * unrestrictedDigitalInformation(2),
	 * audio3100Hz(3),
	 * telephony(32),
	 * teletex(33),
	 * telefaxGroup4Class1(34),
	 * videotextSyntaxBased(35),
	 * videotelephony(36),
	 * telefaxGroup2-3(37),
	 * reservedNotUsed1(38),
	 * reservedNotUsed2(39),
	 * reservedNotUsed3(40),
	 * reservedNotUsed4(41),
	 * reservedNotUsed5(42),
	 * email(51),
	 * video(52),
	 * fileTransfer(53),
	 * shortMessageService(54),
	 * speechAndVideo(55),
	 * speechAndFax(56),
	 * speechAndEmail(57),
	 * videoAndFax(58),
	 * videoAndEmail(59),
	 * faxAndEmail(60),
	 * speechVideoAndFax(61),
	 * speechVideoAndEmail(62),
	 * speechFaxAndEmail(63),
	 * videoFaxAndEmail(64),
	 * speechVideoFaxAndEmail(65),
	 * multimediaUnknown(66),
	 * serviceUnknown(67),
	 * futureReserve1(68),
	 * futureReserve2(69),
	 * futureReserve3(70),
	 * futureReserve4(71),
	 * futureReserve5(72),
	 * futureReserve6(73),
	 * futureReserve7(74),
	 * futureReserve8(75)
	 */
	u_int8_t basic_service;

	/*!
	 * \brief INTEGER (0..9) (optional)
	 * \note The value 0 means the highest priority and 9 the lowest.
	 */
	u_int8_t priority;

	/*! \brief TRUE if msg_centre_id is present */
	u_int8_t msg_centre_id_present;

	/*! \brief TRUE if number_of_messages is present */
	u_int8_t number_of_messages_present;

	/*! \brief TRUE if timestamp is present */
	u_int8_t timestamp_present;

	/*! \brief TRUE if priority is present */
	u_int8_t priority_present;
};

/*
 * MWIInterrogateRes ::= SEQUENCE SIZE(1..10) OF MWIInterrogateResElt
 */
struct roseQsigMWIInterrogateRes {
	/*! \brief SEQUENCE SIZE(1..10) OF MWIInterrogateResElt */
	struct roseQsigMWIInterrogateResElt list[10];

	/*! \brief Number of MWIInterrogateResElt records present */
	u_int8_t num_records;
};


/* ------------------------------------------------------------------- */


/*
 * Northern Telecom DMS-100 transfer ability result
 *
 * callId [0] IMPLICIT INTEGER (0..16777215) -- 24 bit number
 */
struct roseDms100RLTOperationInd_RES {
	/*! INTEGER (0..16777215) -- 24 bit number */
	u_int32_t call_id;
};

/*
 * Northern Telecom DMS-100 transfer invoke
 *
 * ARGUMENT SEQUENCE {
 *     callId [0] IMPLICIT INTEGER (0..16777215), -- 24 bit number
 *     reason [1] IMPLICIT INTEGER
 * }
 */
struct roseDms100RLTThirdParty_ARG {
	/*! INTEGER (0..16777215) -- 24 bit number */
	u_int32_t call_id;

	/*! Reason for redirect */
	u_int8_t reason;
};


/* ------------------------------------------------------------------- */


/* ARGUMENT ENUMERATED */
struct roseNi2InformationFollowing_ARG {
	u_int8_t value;				/*!< Unknown enumerated value */
};

/*
 * ARGUMENT SEQUENCE {
 *     callReference INTEGER -- 16 bit number
 * }
 */
struct roseNi2InitiateTransfer_ARG {
	u_int16_t call_reference;
};


/* ------------------------------------------------------------------- */


/*! \brief Facility ie invoke etsi messages with arguments. */
union rose_msg_invoke_etsi_args {
	/* ETSI Advice Of Charge (AOC) */
	struct roseEtsiChargingRequest_ARG ChargingRequest;
	struct roseEtsiAOCSCurrency_ARG AOCSCurrency;
	struct roseEtsiAOCSSpecialArr_ARG AOCSSpecialArr;
	struct roseEtsiAOCDCurrency_ARG AOCDCurrency;
	struct roseEtsiAOCDChargingUnit_ARG AOCDChargingUnit;
	struct roseEtsiAOCECurrency_ARG AOCECurrency;
	struct roseEtsiAOCEChargingUnit_ARG AOCEChargingUnit;

	/* ETSI Call Diversion */
	struct roseEtsiActivationDiversion_ARG ActivationDiversion;
	struct roseEtsiDeactivationDiversion_ARG DeactivationDiversion;
	struct roseEtsiActivationStatusNotificationDiv_ARG ActivationStatusNotificationDiv;
	struct roseEtsiDeactivationStatusNotificationDiv_ARG
		DeactivationStatusNotificationDiv;
	struct roseEtsiInterrogationDiversion_ARG InterrogationDiversion;
	struct roseEtsiDiversionInformation_ARG DiversionInformation;
	struct roseEtsiCallDeflection_ARG CallDeflection;
	struct roseEtsiCallRerouting_ARG CallRerouting;
	struct roseEtsiDivertingLegInformation1_ARG DivertingLegInformation1;
	struct roseEtsiDivertingLegInformation2_ARG DivertingLegInformation2;
	struct roseEtsiDivertingLegInformation3_ARG DivertingLegInformation3;

	/* ETSI Explicit Call Transfer (ECT) */
	struct roseEtsiExplicitEctExecute_ARG ExplicitEctExecute;
	struct roseEtsiSubaddressTransfer_ARG SubaddressTransfer;
	struct roseEtsiEctInform_ARG EctInform;
	struct roseEtsiEctLoopTest_ARG EctLoopTest;

	/* ETSI Status Request (CCBS/CCNR support) */
	struct roseEtsiStatusRequest_ARG StatusRequest;

	/* ETSI CCBS/CCNR support */
	struct roseEtsiCallInfoRetain_ARG CallInfoRetain;
	struct roseEtsiEraseCallLinkageID_ARG EraseCallLinkageID;
	struct roseEtsiCCBSDeactivate_ARG CCBSDeactivate;
	struct roseEtsiCCBSErase_ARG CCBSErase;
	struct roseEtsiCCBSRemoteUserFree_ARG CCBSRemoteUserFree;
	struct roseEtsiCCBSCall_ARG CCBSCall;
	struct roseEtsiCCBSStatusRequest_ARG CCBSStatusRequest;
	struct roseEtsiCCBSBFree_ARG CCBSBFree;
	struct roseEtsiCCBSStopAlerting_ARG CCBSStopAlerting;

	/* ETSI CCBS */
	struct roseEtsiCCBSRequest_ARG CCBSRequest;
	struct roseEtsiCCBSInterrogate_ARG CCBSInterrogate;

	/* ETSI CCNR */
	struct roseEtsiCCBSRequest_ARG CCNRRequest;
	struct roseEtsiCCBSInterrogate_ARG CCNRInterrogate;

	/* ETSI CCBS-T */
	struct roseEtsiCCBS_T_Request_ARG CCBS_T_Request;

	/* ETSI CCNR-T */
	struct roseEtsiCCBS_T_Request_ARG CCNR_T_Request;

	/* ETSI Message Waiting Indication (MWI) */
	struct roseEtsiMWIActivate_ARG MWIActivate;
	struct roseEtsiMWIDeactivate_ARG MWIDeactivate;
	struct roseEtsiMWIIndicate_ARG MWIIndicate;
};

/*! \brief Facility ie result etsi messages with arguments. */
union rose_msg_result_etsi_args {
	/* ETSI Advice Of Charge (AOC) */
	struct roseEtsiChargingRequest_RES ChargingRequest;

	/* ETSI Call Diversion */
	struct roseEtsiForwardingList InterrogationDiversion;
	struct roseEtsiServedUserNumberList InterrogateServedUserNumbers;

	/* ETSI Explicit Call Transfer (ECT) */
	struct roseEtsiEctLinkIdRequest_RES EctLinkIdRequest;
	struct roseEtsiEctLoopTest_RES EctLoopTest;

	/* ETSI Status Request (CCBS/CCNR support) */
	struct roseEtsiStatusRequest_RES StatusRequest;

	/* ETSI CCBS/CCNR support */
	struct roseEtsiCCBSStatusRequest_RES CCBSStatusRequest;

	/* ETSI CCBS */
	struct roseEtsiCCBSRequest_RES CCBSRequest;
	struct roseEtsiCCBSInterrogate_RES CCBSInterrogate;

	/* ETSI CCNR */
	struct roseEtsiCCBSRequest_RES CCNRRequest;
	struct roseEtsiCCBSInterrogate_RES CCNRInterrogate;

	/* ETSI CCBS-T */
	struct roseEtsiCCBS_T_Request_RES CCBS_T_Request;

	/* ETSI CCNR-T */
	struct roseEtsiCCBS_T_Request_RES CCNR_T_Request;
};

/*! \brief Facility ie invoke qsig messages with arguments. */
union rose_msg_invoke_qsig_args {
	/* Q.SIG Name-Operations */
	struct roseQsigPartyName_ARG CallingName;
	struct roseQsigPartyName_ARG CalledName;
	struct roseQsigPartyName_ARG ConnectedName;
	struct roseQsigPartyName_ARG BusyName;

	/* Q.SIG SS-AOC-Operations */
	struct roseQsigChargeRequestArg_ARG ChargeRequest;
	struct roseQsigAocFinalArg_ARG AocFinal;
	struct roseQsigAocInterimArg_ARG AocInterim;
	struct roseQsigAocRateArg_ARG AocRate;
	struct roseQsigAocCompleteArg_ARG AocComplete;
	struct roseQsigAocDivChargeReqArg_ARG AocDivChargeReq;

	/* Q.SIG Call-Transfer-Operations */
	struct roseQsigCTInitiateArg_ARG CallTransferInitiate;
	struct roseQsigCTSetupArg_ARG CallTransferSetup;
	struct roseQsigCTActiveArg_ARG CallTransferActive;
	struct roseQsigCTCompleteArg_ARG CallTransferComplete;
	struct roseQsigCTUpdateArg_ARG CallTransferUpdate;
	struct roseQsigSubaddressTransferArg_ARG SubaddressTransfer;

	/* Q.SIG Call-Diversion-Operations */
	struct roseQsigActivateDiversionQ_ARG ActivateDiversionQ;
	struct roseQsigDeactivateDiversionQ_ARG DeactivateDiversionQ;
	struct roseQsigInterrogateDiversionQ_ARG InterrogateDiversionQ;
	struct roseQsigCheckRestriction_ARG CheckRestriction;
	struct roseQsigCallRerouting_ARG CallRerouting;
	struct roseQsigDivertingLegInformation1_ARG DivertingLegInformation1;
	struct roseQsigDivertingLegInformation2_ARG DivertingLegInformation2;
	struct roseQsigDivertingLegInformation3_ARG DivertingLegInformation3;

	/* Q.SIG SS-CC-Operations */
	struct roseQsigCcRequestArg CcbsRequest;
	struct roseQsigCcRequestArg CcnrRequest;
	struct roseQsigCcOptionalArg CcCancel;
	struct roseQsigCcOptionalArg CcExecPossible;

	/* Q.SIG SS-MWI-Operations */
	struct roseQsigMWIActivateArg MWIActivate;
	struct roseQsigMWIDeactivateArg MWIDeactivate;
	struct roseQsigMWIInterrogateArg MWIInterrogate;
};

/*! \brief Facility ie result qsig messages with arguments. */
union rose_msg_result_qsig_args {
	/* Q.SIG SS-AOC-Operations */
	struct roseQsigChargeRequestRes_RES ChargeRequest;
	struct roseQsigAocCompleteRes_RES AocComplete;

	/* Q.SIG Call-Transfer-Operations */
	struct roseQsigCTIdentifyRes_RES CallTransferIdentify;

	/* Q.SIG Call-Diversion-Operations */
	struct roseQsigForwardingList InterrogateDiversionQ;

	/* Q.SIG SS-CC-Operations */
	struct roseQsigCcRequestRes CcbsRequest;
	struct roseQsigCcRequestRes CcnrRequest;

	/* Q.SIG SS-MWI-Operations */
	struct roseQsigMWIInterrogateRes MWIInterrogate;
};

/*! \brief Facility ie invoke DMS-100 messages with arguments. */
union rose_msg_invoke_dms100_args {
	struct roseDms100RLTThirdParty_ARG RLT_ThirdParty;
};

/*! \brief Facility ie result DMS-100 messages with arguments. */
union rose_msg_result_dms100_args {
	struct roseDms100RLTOperationInd_RES RLT_OperationInd;
};

/*! \brief Facility ie invoke NI2 messages with arguments. */
union rose_msg_invoke_ni2_args {
	struct roseNi2InformationFollowing_ARG InformationFollowing;
	struct roseNi2InitiateTransfer_ARG InitiateTransfer;
};

/*! \brief Facility ie result NI2 messages with arguments. */
union rose_msg_result_ni2_args {
	int dummy;					/*!< place holder until there are results with parameters */
};

/*! \brief Facility ie invoke messages with arguments. */
union rose_msg_invoke_args {
	union rose_msg_invoke_etsi_args etsi;
	union rose_msg_invoke_qsig_args qsig;
	union rose_msg_invoke_dms100_args dms100;
	union rose_msg_invoke_ni2_args ni2;
};

/*! \brief Facility ie result messages with arguments. */
union rose_msg_result_args {
	union rose_msg_result_etsi_args etsi;
	union rose_msg_result_qsig_args qsig;
	union rose_msg_result_dms100_args dms100;
	union rose_msg_result_ni2_args ni2;
};

/*! \brief Facility ie error messages with parameters. */
union rose_msg_error_args {
	int dummy;					/*!< place holder until there are errors with parameters */
};

struct rose_msg_invoke {
	/*! \brief Invoke ID (-32768..32767) */
	int16_t invoke_id;
	/*! \brief Linked ID (-32768..32767) (optional) */
	int16_t linked_id;
	/*! \brief library encoded operation-value */
	enum rose_operation operation;
	/*! \brief TRUE if the Linked ID is present */
	u_int8_t linked_id_present;
	union rose_msg_invoke_args args;
};

struct rose_msg_result {
	/*! \brief Invoke ID (-32768..32767) */
	int16_t invoke_id;
	/*!
	 * \brief library encoded operation-value
	 * \note Set to ROSE_None if the operation sequence is not present.
	 * \note ETSI and Q.SIG imply that if a return result does not have
	 * any arguments then you must rely upon the invokeId value to
	 * distinguish between return results because the operation-value is
	 * not present.
	 */
	enum rose_operation operation;
	union rose_msg_result_args args;
};

struct rose_msg_error {
	/*! \brief Invoke ID (-32768..32767) */
	int16_t invoke_id;
	/*! \brief library encoded error-value */
	enum rose_error_code code;
	union rose_msg_error_args args;
};

struct rose_msg_reject {
	/*! \brief Invoke ID (-32768..32767) (optional) */
	int16_t invoke_id;
	/*! \brief TRUE if the Invoke ID is present */
	u_int8_t invoke_id_present;
	/*! \brief library encoded problem-value */
	enum rose_reject_code code;
};

enum rose_component_type {
	ROSE_COMP_TYPE_INVALID,
	ROSE_COMP_TYPE_INVOKE,
	ROSE_COMP_TYPE_RESULT,
	ROSE_COMP_TYPE_ERROR,
	ROSE_COMP_TYPE_REJECT
};

struct rose_message {
	/*! \brief invoke, result, error, reject */
	enum rose_component_type type;
	union {
		struct rose_msg_invoke invoke;
		struct rose_msg_result result;
		struct rose_msg_error error;
		struct rose_msg_reject reject;
	} component;
};

/*
 * NetworkFacilityExtension ::= [10] IMPLICIT SEQUENCE {
 *     sourceEntity                [0] IMPLICIT EntityType,
 *     sourceEntityAddress         [1] EXPLICIT AddressInformation OPTIONAL,
 *     destinationEntity           [2] IMPLICIT EntityType,
 *     destinationEntityAddress    [3] EXPLICIT AddressInformation OPTIONAL
 * }
 *
 * AddressInformation ::= PartyNumber
 */
struct facNetworkFacilityExtension {
	/*! \brief sourceEntityAddress (optional) (Number present if length is nonzero) */
	struct rosePartyNumber source_number;
	/*! \brief destinationEntityAddress (optional) (Number present if length is nonzero) */
	struct rosePartyNumber destination_number;

	/*!
	 * \details
	 * endPINX(0),
	 * anyTypeOfPINX(1)
	 */
	u_int8_t source_entity;

	/*!
	 * \details
	 * endPINX(0),
	 * anyTypeOfPINX(1)
	 */
	u_int8_t destination_entity;
};

/*
 * The network extensions header is a sequence of the following components:
 *
 * nfe NetworkFacilityExtension OPTIONAL,
 * npp NetworkProtocolProfile OPTIONAL,
 * interpretation InterpretationApdu OPTIONAL
 *
 * NetworkProtocolProfile ::= [18] IMPLICIT INTEGER (0..254)
 *
 * InterpretationApdu ::= [11] IMPLICIT ENUMERATED {
 *     discardAnyUnrecognisedInvokePdu(0),
 *
 *     -- this value also applies to Call independent signalling connections
 *     -- see clause 8.1.2 (ECMA-165)
 *     clearCallIfAnyInvokePduNotRecognised(1),
 *
 *     -- this coding is implied by the absence of an
 *     -- interpretation APDU.
 *     rejectAnyUnrecognisedInvokePdu(2)
 * }
 */
struct fac_extension_header {
	/*! \brief Network Facility Extension component */
	struct facNetworkFacilityExtension nfe;

	/*! \brief Network Protocol Profile component */
	u_int8_t npp;

	/*!
	 * \brief interpretation component
	 *
	 * \details
	 * discardAnyUnrecognisedInvokePdu(0),
	 * clearCallIfAnyInvokePduNotRecognised(1),
	 * rejectAnyUnrecognisedInvokePdu(2)
	 */
	u_int8_t interpretation;

	/*! \brief TRUE if nfe is present */
	u_int8_t nfe_present;

	/*! \brief TRUE if npp is present */
	u_int8_t npp_present;

	/*! \brief TRUE if interpretation is present */
	u_int8_t interpretation_present;
};

const char *rose_operation2str(enum rose_operation operation);
const char *rose_error2str(enum rose_error_code code);
const char *rose_reject2str(enum rose_reject_code code);

unsigned char *rose_encode_invoke(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct rose_msg_invoke *msg);
unsigned char *rose_encode_result(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct rose_msg_result *msg);
unsigned char *rose_encode_error(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct rose_msg_error *msg);
unsigned char *rose_encode_reject(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct rose_msg_reject *msg);

unsigned char *rose_encode(struct pri *ctrl, unsigned char *pos, unsigned char *end,
	const struct rose_message *msg);
const unsigned char *rose_decode(struct pri *ctrl, const unsigned char *pos,
	const unsigned char *end, struct rose_message *msg);

unsigned char *fac_enc_extension_header(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct fac_extension_header *header);
unsigned char *facility_encode_header(struct pri *ctrl, unsigned char *pos,
	unsigned char *end, const struct fac_extension_header *header);

const unsigned char *fac_dec_extension_header(struct pri *ctrl, const unsigned char *pos,
	const unsigned char *end, struct fac_extension_header *header);
const unsigned char *facility_decode_header(struct pri *ctrl, const unsigned char *pos,
	const unsigned char *end, struct fac_extension_header *header);

void facility_decode_dump(struct pri *ctrl, const unsigned char *buf, size_t length);

/* ------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif	/* _LIBPRI_ROSE_H */
/* ------------------------------------------------------------------- */
/* end rose.h */
