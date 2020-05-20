#include "transaction.h"
#include "account.h"
#include <QDateTime>

quint32 Transaction::Builder::s_defaultOperationFee = Transaction::Builder::BASE_FEE;

Transaction::Transaction(QString sourceAccount, qint64 fee, qint64 sequenceNumber, QVector<Operation *> operations, Memo *memo, TimeBounds *timeBounds, Network *network)
    :AbstractTransaction(network),m_envelopeType(stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0)
{
    m_sourceAccount = checkNotNull(sourceAccount, "sourceAccount cannot be null");
    m_sequenceNumber=sequenceNumber;//we cant check this, all the values are valid


    checkArgument(operations.length() > 0, "At least one operation required");
    m_operations = operations;
    m_fee = fee;
    m_memo =(memo) ? memo : Memo::none();
    m_timeBounds = timeBounds;
}

Transaction::~Transaction(){
    m_sourceAccount.fill('\0');
    if(m_memo)
        delete m_memo;
    if(m_timeBounds)
        delete m_timeBounds;
    for(Operation * o : m_operations){
        delete o;
    }
}



QByteArray Transaction::signatureBase() const{
    try {

        QByteArray output;
        QDataStream outputStream(&output,QIODevice::WriteOnly);
        stellar::TransactionSignaturePayload payload(this->toV1Xdr(), m_network->getNetworkId());
        outputStream << payload;
        return output;
    } catch (std::exception e) {
        return QByteArray();
    }
}

QString Transaction::getSourceAccount() const {
    return m_sourceAccount;
}

qint64 Transaction::getSequenceNumber() const{
    return m_sequenceNumber;
}

Memo *Transaction::getMemo() const
{
    return m_memo;
}

Network *Transaction::getNetwork() const
{
    return m_network;
}

TimeBounds *Transaction::getTimeBounds() const
{
    return m_timeBounds;
}

QVector<Operation *> Transaction::getOperations() const{
    return m_operations;
}

qint64 Transaction::getFee() const{
    return m_fee;
}

stellar::TransactionV0 Transaction::toV0Xdr() const{

    stellar::TransactionV0 transaction;

    transaction.memo = m_memo->toXdr();
    if(m_timeBounds)
    {
        stellar::TimeBounds& tm = transaction.timeBounds.filler();
        tm.minTime=static_cast<quint64>(m_timeBounds->getMinTime());
        tm.maxTime=static_cast<quint64>(m_timeBounds->getMaxTime());
    }
    // fee
    transaction.fee = m_fee;
    // sequenceNumber
    transaction.seqNum = m_sequenceNumber;
    // sourceAccount    
    stellar::AccountID accountID = StrKey::encodeToXDRAccountId(m_sourceAccount);
    memcpy(transaction.sourceAccountEd25519,accountID.ed25519,sizeof(transaction.sourceAccountEd25519));
    // operations
    for (int i = 0; i < m_operations.length(); i++) {
        transaction.operations.append(m_operations.at(i)->toXdr());
    }
    return transaction;
}

stellar::Transaction Transaction::toV1Xdr() const{

    stellar::Transaction transaction;

    transaction.memo = m_memo->toXdr();
    if(m_timeBounds)
    {
        stellar::TimeBounds& tm = transaction.timeBounds.filler();
        tm.minTime=static_cast<quint64>(m_timeBounds->getMinTime());
        tm.maxTime=static_cast<quint64>(m_timeBounds->getMaxTime());
    }
    // fee
    transaction.fee = m_fee;
    // sequenceNumber
    transaction.seqNum = m_sequenceNumber;
    // sourceAccount
    transaction.sourceAccount =StrKey::encodeToXDRMuxedAccount(m_sourceAccount);
    // operations
    for (int i = 0; i < m_operations.length(); i++) {
        transaction.operations.append(m_operations.at(i)->toXdr());
    }
    return transaction;
}

Transaction *Transaction::fromV0EnvelopeXdr(stellar::TransactionV0Envelope &envelope, Network *network)
{
    QString sourceAccount = StrKey::encodeStellarAccountId(envelope.tx.sourceAccountEd25519);
    QVector<Operation*> ops;
    for(auto op : envelope.tx.operations.value)
    {
        ops.append(Operation::fromXdr(op));
    }
    Transaction *t = new Transaction(sourceAccount,envelope.tx.fee,envelope.tx.seqNum,ops,Memo::fromXdr(envelope.tx.memo), envelope.tx.timeBounds.filled ? TimeBounds::fromXdr(envelope.tx.timeBounds.value) : nullptr, network);
    t->m_envelopeType = stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0;
    for (stellar::DecoratedSignature& signature : envelope.signatures.value) {
        t->m_signatures.append(signature);
    }
    return t;

}

Transaction *Transaction::fromV1EnvelopeXdr(stellar::TransactionV1Envelope &envelope, Network *network)
{
    QString sourceAccount = StrKey::encodeStellarMuxedAccount(envelope.tx.sourceAccount);
    QVector<Operation*> ops;
    for(auto op : envelope.tx.operations.value)
    {
        ops.append(Operation::fromXdr(op));
    }
    Transaction * t = new Transaction(sourceAccount,envelope.tx.fee,envelope.tx.seqNum,ops,Memo::fromXdr(envelope.tx.memo), envelope.tx.timeBounds.filled ? TimeBounds::fromXdr(envelope.tx.timeBounds.value) : nullptr, network);
    t->m_envelopeType = stellar::EnvelopeType::ENVELOPE_TYPE_TX;
    for (stellar::DecoratedSignature& signature : envelope.signatures.value) {
        t->m_signatures.append(signature);
    }
    return t;
}


stellar::TransactionEnvelope Transaction::toEnvelopeXdr(){
    switch(m_envelopeType)
    {
    case stellar::EnvelopeType::ENVELOPE_TYPE_TX:
    {
        stellar::TransactionV1Envelope envelope;        
        envelope.tx=toV1Xdr();
        for(stellar::DecoratedSignature& signature : this->m_signatures){
            envelope.signatures.append(signature);
        }
        return stellar::TransactionEnvelope(envelope);
    }
    case stellar::EnvelopeType::ENVELOPE_TYPE_TX_V0:
    {
        stellar::TransactionV0Envelope envelope;
        envelope.tx=toV0Xdr();
        for(stellar::DecoratedSignature& signature : this->m_signatures){
            envelope.signatures.append(signature);
        }
        return stellar::TransactionEnvelope(envelope);
        break;
    }
    default:
    {
        throw std::runtime_error("invalid envelope type");
    }
    }
}

Transaction::Builder::Builder(TransactionBuilderAccount *sourceAccount, Network *network) {
    m_sourceAccount = checkNotNull(sourceAccount, "sourceAccount cannot be null");
    m_memo=nullptr;
    m_timeBounds=nullptr;
#ifdef STELLAR_QT_AUTOSET_BASE_FEE
    m_baseFee = s_defaultOperationFee;
#else
    m_baseFee = 0;
#endif
    m_timeoutSet=false;
    m_network= network;
}

Transaction::Builder::~Builder()
{
    //it will not destroy the account object
    if(m_memo)
        delete m_memo;
    if(m_timeBounds)
        delete m_timeBounds;
    for(Operation * o : m_operations){
        delete o;
    }
}

int Transaction::Builder::getOperationsCount() {
    return m_operations.size();
}

void Transaction::Builder::setDefaultOperationFee(quint32 opFee) {
    if (opFee < Builder::BASE_FEE) {
        throw std::runtime_error(QString("DefaultOperationFee cannot be smaller than the BASE_FEE (\" %1 \"): %2").arg(Builder::BASE_FEE).arg(opFee).toStdString());
    }
    s_defaultOperationFee = opFee;
}

Transaction::Builder &Transaction::Builder::addOperation(Operation *operation) {
    checkNotNull(operation, "operation cannot be null");
    m_operations.append(operation);
    return *this;
}

Transaction::Builder &Transaction::Builder::addMemo(Memo *memo) {
    if (this->m_memo) {
        throw std::runtime_error("Memo has been already added.");
    }
    m_memo = checkNotNull(memo, "memo cannot be null");
    return *this;
}

Transaction::Builder &Transaction::Builder::addTimeBounds(TimeBounds *timeBounds) {
    if (this->m_timeBounds) {
        throw std::runtime_error("TimeBounds has been already added.");
    }
    checkNotNull(reinterpret_cast<intptr_t>(timeBounds), "timeBounds cannot be null");
    m_timeBounds = timeBounds;
    m_timeoutSet = true;
    return *this;
}

Transaction::Builder &Transaction::Builder::setTimeout(qint64 timeout) {
    if (m_timeBounds && m_timeBounds->getMaxTime() > 0) {
        throw std::runtime_error("TimeBounds.max_time has been already set - setting timeout would overwrite it.");
    }

    if (timeout < 0) {
        throw std::runtime_error("timeout cannot be negative");
    }

    m_timeoutSet = true;
    if (timeout > 0) {
        qint64 timeoutTimestamp = QDateTime::currentMSecsSinceEpoch()/ 1000L + timeout;
        if (!m_timeBounds) {
            m_timeBounds = new TimeBounds(0, timeoutTimestamp);
        } else {
            qint64 min = m_timeBounds->getMinTime();
            delete m_timeBounds;
            m_timeBounds = new TimeBounds(min, timeoutTimestamp);
        }
    }

    return *this;
}

Transaction::Builder &Transaction::Builder::setBaseFee(quint32 baseFee) {
    if (baseFee < Builder::BASE_FEE) {
        throw std::runtime_error(QString("BaseFee cannot be smaller than the BASE_FEE (\" %1 \"): %2").arg(Builder::BASE_FEE).arg(baseFee).toStdString());
    }

    m_baseFee = baseFee;
    return *this;
}

Transaction *Transaction::Builder::build() {
    // Ensure setTimeout called or maxTime is set
    if ((!m_timeBounds || (m_timeBounds  && m_timeBounds->getMaxTime() == 0)) && !m_timeoutSet) {
      throw std::runtime_error("TimeBounds has to be set or you must call setTimeout(TIMEOUT_INFINITE).");
    }

    if (m_baseFee == 0) {
#ifdef STELLAR_QT_AUTOSET_BASE_FEE
        qDebug()<< "[TransactionBuilder] The `baseFee` parameter of `TransactionBuilder` is required. Setting to BASE_FEE=" << Builder::BASE_FEE << ". Future versions of this library will error if not provided.";
        m_baseFee = Builder::BASE_FEE;
#else
        throw std::runtime_error("The `baseFee` parameter of `TransactionBuilder` is required.");
#endif
    }    

    Transaction *transaction = new Transaction(m_sourceAccount->getKeypair()->getAccountId()
                                               , static_cast<quint32>(m_operations.length()) * m_baseFee
                                               , m_sourceAccount->getIncrementedSequenceNumber()
                                               , m_operations, m_memo, m_timeBounds,m_network);
    // Increment sequence number when there were no exceptions when creating a transaction
    m_sourceAccount->incrementSequenceNumber();

    //so objects dont get destroyed with the builder, all the checks were done already
    m_memo=nullptr;
    m_timeBounds=nullptr;
    m_operations.clear();

    return transaction;
}

Transaction *checkNotNull(Transaction *transaction, const char *error)
{
    if(!transaction){
        throw std::runtime_error(error);
    }
    return transaction;
}
