package routes

import (
	"OpenFabControl/handler/resource_handler"
	"OpenFabControl/handler/role_handler"
	"OpenFabControl/handler/session_handler"
	"OpenFabControl/handler/user_handler"
	"fmt"
	"net/http"
)

// function to manage routes
func Setup_routes() {
	//////////////////////////////
	// machine controler routes //
	//////////////////////////////

	http.HandleFunc("/machine-api/register", resource_handler.Register)
	http.HandleFunc("/machine-api/check_approval_status", resource_handler.Check_approval_status)
	http.HandleFunc("/machine-api/create_session", session_handler.Create_session)
	http.HandleFunc("/machine-api/start_session", session_handler.Start_session)
	http.HandleFunc("/machine-api/stop_session", session_handler.Stop_session)
	http.HandleFunc("/machine-api/next_booking", session_handler.Next_booking)
	http.HandleFunc("/machine-api/create_user", user_handler.Create_user)

	///////////////////////
	// admin page routes //
	///////////////////////

	// TODO : admin pages have to be protected (not done for the moment for dev purpose)

	// machine controlers
	http.HandleFunc("/web-admin-api/get_resource_list_to_approve", resource_handler.Get_resource_list_to_approve)
	http.HandleFunc("/web-admin-api/get_resource_list_approved", resource_handler.Get_resource_list_approved)
	http.HandleFunc("/web-admin-api/approve_resource", resource_handler.Approve_resource)
	http.HandleFunc("/web-admin-api/delete_resource", resource_handler.Delete_resource)
	http.HandleFunc("/web-admin-api/edit_resource", resource_handler.Edit_resource)
	http.HandleFunc("/web-admin-api/create_session", session_handler.Create_session)

	// users
	http.HandleFunc("/web-admin-api/create_user", user_handler.Create_user)
	http.HandleFunc("/web-admin-api/get_user_list", user_handler.Get_user_list)
	http.HandleFunc("/web-admin-api/delete_user", user_handler.Delete_user)
	http.HandleFunc("/web-admin-api/update_user", user_handler.Update_user)
	http.HandleFunc("/web-admin-api/desactivate_user", user_handler.Desactivate_user)
	http.HandleFunc("/web-admin-api/reactivate_user", user_handler.Reactivate_user)
	http.HandleFunc("/web-admin-api/get_user_roles", user_handler.Get_user_roles)
	// roles
	http.HandleFunc("/web-admin-api/get_role_list", role_handler.Get_role_list)
	http.HandleFunc("/web-admin-api/create_role", role_handler.Create_role)
	http.HandleFunc("/web-admin-api/delete_role", role_handler.Delete_role)
	http.HandleFunc("/web-admin-api/assign_role_to_user", role_handler.Assign_role_to_user)
	http.HandleFunc("/web-admin-api/remove_role_from_user", role_handler.Remove_role_from_user)

	///////////////////////
	// user pages routes //
	///////////////////////

	http.HandleFunc("/web-user-api/user_one_time_setup", user_handler.User_one_time_setup)
	http.HandleFunc("/web-user-api/create_session", session_handler.Create_session)
	http.HandleFunc("/web-user-api/login", user_handler.Login)

	///////////
	// other //
	///////////

	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		fmt.Fprintf(w, "controle server working")
	})
}
