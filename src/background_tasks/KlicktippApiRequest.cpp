#include "KlicktippApiRequest.h"
#include "../main.h"
#include <lithium_http_client.hh>
#include <lithium_metamap.hh>
#include <lithium_json.hh>
#include "../model/Config.h"
#include "../logging/Logging.h"
#include "../view/View.h"

using namespace li;

namespace task {
    
    KlicktippApiRequest::KlicktippApiRequest()
    {

    }

    KlicktippApiRequest::~KlicktippApiRequest()
    {

    }

    void KlicktippApiRequest::run()
    {   
        std::string url = g_Config->gdtServerUrl + "/contacts/apiBulkUpdateKlicktipp";
        auto res = http_post(url, s::post_parameters = mmm(s::json = 
            json_object_vector(s::email, s::euroSum, s::gdtSum).encode(mKlicktippEntries)
        ));
        auto state = mmm(s::state = std::string());
        json_decode(res.body, state);
        if(state.state != "success") {
            LOG_ERROR(res.body);
        } else {
            LOG_DEBUG(res.body);
        }
    }

    void KlicktippApiRequest::addKlicktippEntry(const model::KlicktippEntry& klicktippEntry)
    {
        mKlicktippEntries.push_back(klicktippEntry);
    }
}